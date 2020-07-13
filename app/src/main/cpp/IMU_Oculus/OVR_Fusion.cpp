#include <stdio.h>
#include <stdlib.h>

#include "OVR_Fusion.h"
#include "OVR_DataStructure.h"
#include <sys/system_properties.h>

namespace OVR {

// This is a "perceptually tuned predictive filter", which means that it is optimized
// for improvements in the VR experience, rather than pure error.  In particular,
// jitter is more perceptible at lower speeds whereas latency is more perceptable
// after a high-speed motion.  Therefore, the prediction interval is dynamically
// adjusted based on speed.  Significant more research is needed to further improve
// this family of filters.
    Posef calcPredictedPose(const PoseStatef &poseState, float predictionDt) {
        Posef pose = poseState.Transform;
        const float linearCoef = 1.0;
        Vector3f angularVelocity = poseState.AngularVelocity;
        float angularSpeed = angularVelocity.Length();

        // This could be tuned so that linear and angular are combined with different coefficients
        float speed = angularSpeed + linearCoef * poseState.LinearVelocity.Length();

        const float slope = 0.2; // The rate at which the dynamic prediction interval varies
        float candidateDt = slope * speed; // TODO: Replace with smoothstep function

        float dynamicDt = predictionDt;

        // Choose the candidate if it is shorter, to improve stability
        if (candidateDt < predictionDt)
            dynamicDt = candidateDt;

        const float MAX_DELTA_TIME = 1.0f / 10.0f;
        dynamicDt = Alg::Clamp(dynamicDt, 0.0f, MAX_DELTA_TIME);

        if (angularSpeed > 0.001)
            pose.Orientation = pose.Orientation * Quatf(angularVelocity, angularSpeed * dynamicDt);

        pose.Position += poseState.LinearVelocity * dynamicDt;

        return pose;
    }


// These two functions need to be moved into Quat class
// Compute a rotation required to Pose "from" into "to".
    Quatf vectorAlignmentRotation(const Vector3f &from, const Vector3f &to) {
        Vector3f axis = from.Cross(to);
        if (axis.LengthSq() == 0)
            // this handles both collinear and zero-length input cases
            return Quatf();
        float angle = from.Angle(to);
        return Quatf(axis, angle);
    }

    OcFusion::OcFusion() :
            ApplyDrift(false),
            mSleepToWake(false),
            FAccelHeadset(400),
            FAngV(20),
            MotionTrackingEnabled(true),
            EnableGravity(true),
            EnableYawCorrection(false),
            GeomagneticSwitch(false),
            FocusDirection(Vector3f(0, 0, 0)),
            FocusFOV(0.0),
            RecenterTransform() {
        LOG("SensorDataFusion Constructor ");
        mSleepToWake = false;
        //Reset();
        char calibration_data[20];
        char space;
        int i, j;
        char geomagnetic_switch[2];
        int len;
        len = __system_property_get("persist.ovr.geomagnetic.mode", geomagnetic_switch);
        if (len > 0) {
            int ret = strcmp(geomagnetic_switch, "1");
            if (ret == 0) {
                LOG("geomagnetic_switch = %s", geomagnetic_switch);
                GeomagneticSwitch = true;
            } else {
                LOG("geomagnetic_switch = %s", geomagnetic_switch);
                GeomagneticSwitch = false;
            }
        } else {
            LOG("system_property_get persist.ovr.geomagnetic.mode fail");
            GeomagneticSwitch = false;
        }
    }

    OcFusion::~OcFusion() {
        LOG("SensorDataFusion deConstructor ");
        mSleepToWake = false;
//    delete mpSensorState;
//    mpSensorState = NULL;
    }


    void OcFusion::HandleMessage(const MessageBodyFrame &msg) {
        if (msg.Acceleration == Vector3f::ZERO)
            return;
        // Put the sensor readings into convenient local variables
        Vector3f gyro(msg.RotationRate);
        Vector3f accel(msg.Acceleration);
        Vector3f mag(msg.MagneticField);
        Vector3f magBias(msg.MagneticBias);
        float DeltaT = msg.TimeDelta;

        mag = Vector3f(
                Geomagnetic_calibration[0][0] * mag.x + Geomagnetic_calibration[0][1] * mag.y +
                Geomagnetic_calibration[0][2] * mag.z + Geomagnetic_calibration[0][3],
                Geomagnetic_calibration[1][0] * mag.x + Geomagnetic_calibration[1][1] * mag.y +
                Geomagnetic_calibration[1][2] * mag.z + Geomagnetic_calibration[1][3],
                Geomagnetic_calibration[2][0] * mag.x + Geomagnetic_calibration[2][1] * mag.y +
                Geomagnetic_calibration[2][2] * mag.z + Geomagnetic_calibration[2][3]);

        // Keep track of time
        State.TimeInSeconds = msg.AbsoluteTimeSeconds;
        Stage++;

        // Insert current sensor data into filter history
        FAngV.PushBack(gyro);
        FAccelHeadset.Update(accel, DeltaT, Quatf(gyro, gyro.Length() * DeltaT));

        // Process raw inputs
        State.AngularVelocity = gyro;
        State.LinearAcceleration = State.Transform.Orientation.Rotate(accel) - Vector3f(0, 9.8f, 0);
        // Update headset orientation
        float angle = gyro.Length() * DeltaT;
        if (angle > 0)
            State.Transform.Orientation = State.Transform.Orientation * Quatf(gyro, angle);
        // Tilt correction based on accelerometer
        if (EnableGravity)
            applyTiltCorrection(DeltaT);
        // Yaw correction based on magnetometer
        if (EnableYawCorrection && GeomagneticSwitch)
            applyMagYawCorrection(mag, gyro, DeltaT);
/*
	// Focus Correction
	if ((FocusDirection.x != 0.0f || FocusDirection.z != 0.0f) && FocusFOV < Mathf::Pi)
		applyFocusCorrection(DeltaT);
*/
        // The quaternion magnitude may slowly drift due to numerical error,
        // so it is periodically normalized.
        if ((Stage & 0xFF) == 0)
            State.Transform.Orientation.Normalize();

        // Update headset position
        {
            EnableGravity = true;
//		EnableYawCorrection = true;

            // TBD apply neck model here
            State.LinearVelocity.x = State.LinearVelocity.y = State.LinearVelocity.z = 0.0f;
            State.Transform.Position.x = State.Transform.Position.y = State.Transform.Position.z = 0.0f;
        }

        // Compute the angular acceleration
        State.AngularAcceleration = (FAngV.GetSize() >= 12 && DeltaT > 0) ?
                                    (FAngV.SavitzkyGolayDerivative12() / DeltaT) : Vector3f();
        float yaw, pitch, roll;
        State.Transform.Orientation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
        this->mPitch = abs(pitch / 3.1415926 * 180.0);
        this->mRoll = abs(roll / 3.1415926 * 180.0);

        // Store the lockless state.
        StateForPrediction state;
        state.State = State;
        state.Temperature = msg.Temperature;
        UpdatedState.SetState(state);
#if 0
        const double now = OVR::Timer::GetSeconds();
        static double prev;
        const double rawDelta = now - prev;
        if (rawDelta > 5.0)
        {
            float yaw, pitch, roll;
            State.Transform.Orientation.GetEulerAngles< Axis_Y, Axis_X, Axis_Z >(&yaw, &pitch, &roll);
    //		__android_log_print(ANDROID_LOG_ERROR, "Yaw", "Yaw:%d", abs(yaw / 3.1415926 * 180.0));
            prev = now;
        }
#endif
    }

//  A predictive filter based on extrapolating the smoothed, current angular velocity
    SensorState OcFusion::GetPredictionForTime(const double absoluteTimeSeconds) const {
        SensorState sstate;
        sstate.Status = Status_OrientationTracked | Status_HmdConnected;

        // lockless state fetch
        const StateForPrediction state = UpdatedState.GetState();

        // Delta time from the last processed message
        const float pdt = absoluteTimeSeconds - state.State.TimeInSeconds;

        sstate.Recorded = state.State;
        sstate.Temperature = state.Temperature;

        //const Posef recenter = RecenterTransform.GetState();

        // Do prediction logic
        sstate.Predicted = sstate.Recorded;
        sstate.Predicted.TimeInSeconds = absoluteTimeSeconds;
        sstate.Predicted.Transform = RecenterTransform * calcPredictedPose(state.State, pdt);

        return sstate;
    }

    void OcFusion::applyTiltCorrection(float deltaT) {
        const float gain = 0.25;
        const float snapThreshold = 0.1;
        const Vector3f up(0, 1, 0);

        Vector3f accelLocalFiltered(FAccelHeadset.GetFilteredValue());
        Vector3f accelW = State.Transform.Orientation.Rotate(accelLocalFiltered);
        Quatf error = vectorAlignmentRotation(accelW, up);

        Quatf correction;
        if (FAccelHeadset.GetSize() == 1 ||
            ((Alg::Abs(error.w) < cos(snapThreshold / 2) && FAccelHeadset.Confidence() > 0.75))) {
            // full correction for start-up
            // or large error with high confidence
            correction = error;
        } else if (FAccelHeadset.Confidence() > 0.5) {
            correction = error.Nlerp(Quatf(), gain * deltaT);
        } else {
            // accelerometer is unreliable due to movement
            return;
        }

        State.Transform.Orientation = correction * State.Transform.Orientation;
    }

    void OcFusion::applyMagYawCorrection(Vector3f mag, Vector3f gyro, float deltaT) {
        static int chushi_luke = 0;
        //?y��??��
        const double minMagLengthSq = Mathd::Tolerance; // need to use a real value to discard very weak fields
        double maxMagRefDist = 0.05;
        const double maxTiltError = 0.05;//0.05
        double gyroLength = gyro.Length();

        double proportionalGain = 0;
        const double integralGain = 0.0005;

        if (gyroLength <= 0.5) {
            proportionalGain = 0;
        } else if (gyroLength > 0.5 && gyroLength < 1.0) {
            proportionalGain = 0.01;
        } else if (gyroLength > 1.0) {
            proportionalGain = 0.01 * pow(gyroLength, 2);
        }
        //10.25 end

        //Vector3f magInWorldFrame = WorldFromImu.Pose.Rotate(mag);
        Vector3f magInWorldFrame = State.Transform.Orientation.Rotate(mag);
        // verify that the horizontal component is sufficient
        if (magInWorldFrame.x * magInWorldFrame.x + magInWorldFrame.z * magInWorldFrame.z <
            minMagLengthSq)
            return;
        magInWorldFrame.Normalize();

        // Delete a bad point
        if (MagRefIdx >= 0 && MagRefs[MagRefIdx].Score < 0) {
            MagRefs.RemoveAtUnordered(MagRefIdx);
            MagRefIdx = -1;
        }

        // Update the reference point if needed
        if (MagRefIdx < 0 ||
            mag.Distance(MagRefs[MagRefIdx].MagUncalibratedInImuFrame) > maxMagRefDist) {
            // Find a new one
            MagRefIdx = -1;
            double bestDist = maxMagRefDist * 2.0f;
            for (unsigned int i = 0; i < MagRefs.GetSize(); i++) {
                double dist = mag.Distance(MagRefs[i].MagUncalibratedInImuFrame);
                if (bestDist > dist) {
                    bestDist = dist;
                    MagRefIdx = i;
                }
            }

            // Create one if needed
            if (MagRefIdx < 0 && MagRefs.GetSize() < 500) {
                MagRefs.PushBack(MagReferencePoint(mag, State.Transform.Orientation, 60));
            }
        }

        if (MagRefIdx >= 0) {
            Vector3f magRefInWorldFrame = MagRefs[MagRefIdx].WorldFromImu.Rotate(
                    MagRefs[MagRefIdx].MagUncalibratedInImuFrame);
            magRefInWorldFrame.Normalize();

            // If the vertical angle is wrong, decrease the score and do nothing
            if (Alg::Abs(magRefInWorldFrame.y - magInWorldFrame.y) > maxTiltError) {
                MagRefs[MagRefIdx].Score -= 1;
                return;
            }

            MagRefs[MagRefIdx].Score += 2;//2
#if 0
            // this doesn't seem to work properly, need to investigate
            Quatf error = vectorAlignmentRotation(magW, magRefW);
            Quatf yawError = extractYawRotation(error);
#else
            // Correction is computed in the horizontal plane
            magInWorldFrame.y = magRefInWorldFrame.y = 0;
            Quatf yawError = vectorAlignmentRotation(magInWorldFrame, magRefInWorldFrame);
#endif
            //Quatf correction = yawError.Nlerp(Quatf(), proportionalGain * deltaT) *
            //MagCorrectionIntegralTermQ.Nlerp(Quatf(), deltaT);
            //MagCorrectionIntegralTermQ = MagCorrectionIntegralTermQ * yawError.Nlerp(Quatf(), integralGain * deltaT);
            Quatf correction = yawError.Nlerp(Quatf(), proportionalGain * deltaT);

            //WorldFromImu.Pose.Rotation = correction * WorldFromImu.Pose.Rotation;
            State.Transform.Orientation = correction * State.Transform.Orientation;
        }
    }

    void OcFusion::Reset() {
        //Lock::Locker lockScope(pHandler->GetHandlerLock());
        //UpdatedState.SetState(StateForPrediction());
        if (mSleepToWake == false) {
            LOG("SensorFusion::SleepToWake ---> Reset");
            State = PoseStatef();
            Stage = 0;

            MagRefs.Clear();
            MagRefIdx = -1;
            MagCorrectionIntegralTerm = 0.0f;
            MagLatencyCompBufferIndex = 0;
            MagLatencyCompFillCount = 0;
            YawCorrectionTimer = 0.0f;

            FAccelHeadset.Clear();
            FAngV.Clear();

            RecenterTransform = Posef();
            mSleepToWake = true;
        } else {
            if (this->mPitch < 10 && this->mRoll < 10)
                RecenterYaw();
            else
                RecenterOrientation();
        }
    }

    void OcFusion::RecenterYaw() {
        LOG("SensorFusion::RecenterYaw()");

        // get the current state
        const StateForPrediction state = UpdatedState.GetState();

        // get the yaw in the current state
        float yaw, pitch, roll;
        state.State.Transform.Orientation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch,
                                                                                 &roll);

        // get the pose that adjusts the yaw
        Posef yawAdjustment(Quatf(Axis_Y, yaw), Vector3f(0.0f));

        // To allow RecenterYaw() to be called from multiple threads we need a mutex
        // because LocklessUpdater is only safe for single producer cases.
//	RecenterMutex.DoLock();
//	RecenterTransform.SetState( yawAdjustment );
//	RecenterMutex.Unlock();
        RecenterTransform = yawAdjustment.Inverted();

    }

    void OcFusion::RecenterOrientation() {
        LOG("SensorFusion::RecenterOrientation()");
        // get the current state
        const StateForPrediction state = UpdatedState.GetState();

        Vector3f translation(
                0.0f);    // there is no positional tracking yet, so we don't recenter position
        Posef pose(state.State.Transform.Orientation, translation);
        //RecenterTransform.SetState( pose.Inverted() );
        RecenterTransform = pose.Inverted();
    }

    float OcFusion::GetYaw() {
        // get the current state
        const StateForPrediction state = UpdatedState.GetState();

        // get the yaw in the current state
        float yaw, pitch, roll;
        state.State.Transform.Orientation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch,
                                                                                 &roll);

        return yaw;
    }


    void OcFusion::SetYaw(float newYaw) {
        // get the current state
        const StateForPrediction state = UpdatedState.GetState();

        // get the yaw in the current state
        float yaw, pitch, roll;
        state.State.Transform.Orientation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch,
                                                                                 &roll);

        // get the pose that adjusts the yaw
        Posef yawAdjustment(Quatf(Axis_Y, newYaw - yaw), Vector3f(0.0f));

        // To allow SetYaw() to be called from multiple threads we need a mutex
        // because LocklessUpdater is only safe for single producer cases.
//	RecenterMutex.DoLock();
//	RecenterTransform.SetState( yawAdjustment );
//	RecenterMutex.Unlock();
    }


}

