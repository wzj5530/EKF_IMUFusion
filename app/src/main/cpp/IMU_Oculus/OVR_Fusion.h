#ifndef OC_FUSION_H
#define OC_FUSION_H

#include "OVR_SensorFilter.h"
#include "OVR_Lockless.h"
#include "OVR_Array.h"
#include "OVR_Threads.h"
#include "OVR_Allocator.h"
#include "OVR_DataStructure.h"
#include "OVR_SensorTypes.h"
#include "LogUtils.h"
#include "OVR_Timer.h"

namespace OVR {

//-------------------------------------------------------------------------------------
// ***** SensorFusion

// SensorFusion class accumulates Sensor notification messages to keep track of
// orientation, which involves integrating the gyro and doing correction with gravity.
// Magnetometer based yaw drift correction is also supported; it is usually enabled
// automatically based on loaded magnetometer configuration.
// Orientation is reported as a quaternion, from which users can obtain either the
// rotation matrix or Euler angles.
//
// The class can operate in two ways:
//  - By user manually passing MessageBodyFrame messages to the OnMessage() function. 
//  - By attaching SensorFusion to a SensorDevice, in which case it will
//    automatically handle notifications from that device.

//class OcFusion : public NewOverrideBase

    class OcFusion {
        enum {
            MagMaxReferences = 1000,
            MagLatencyBufferSizeMax = 512,
            MagLatencyCompensationMilliseconds = 95,
        };

    public:
        OcFusion();

        ~OcFusion();

        bool ApplyDrift;
        bool mSleepToWake;
        float mPitch;
        float mRoll;
        float Geomagnetic_calibration[4][4] = {
                {0.021548, 0,       0,       0.02404},
                {0,        0.02205, 0,       -0.01879},
                {0,        0,       0.02142, 0.004219},
                {0,        0,       0,       1}
        };
//    Matrix4x4
        // *** Setup

        // Attaches this SensorFusion to a sensor device, from which it will receive
        // notification messages. If a sensor is attached, manual message notification
        // is not necessary. Calling this function also resets SensorFusion state.
        bool AttachToSensor();

        // Returns true if this Sensor fusion object is attached to a sensor.
//    bool        IsAttachedToSensor() const  { return pHandler->IsHandlerInstalled(); }

        // Resets everything.
        void Reset();


        // *** State Query - These can be called any time from any thread.

        // The orientation will be predicted to the given time from the base of the most
        // recently processed messages.  In general, absoluteTimeSeconds should always be
        // ahead of the most recent message time, but it is possible for a new messages to
        // arrive right before processing, giving a small negative delta in rare cases.
        SensorState GetPredictionForTime(double absoluteTimeSeconds) const;

        // Get the current yaw.
        float GetYaw();

        // Set a particular yaw.
        void SetYaw(float newYaw);

        // Recenters the current orientation on yaw axis
        void RecenterYaw();

        void RecenterOrientation();


        // *** Configuration

        void EnableMotionTracking(bool enable = true) { MotionTrackingEnabled = enable; }

        bool IsMotionTrackingEnabled() const { return MotionTrackingEnabled; }



        // *** Accelerometer/Gravity Correction Control

        // Enables/disables gravity correction (on by default).
        void SetGravityEnabled(bool enableGravity) { EnableGravity = enableGravity; }

        bool IsGravityEnabled() const { return EnableGravity; }


        // *** Magnetometer and Yaw Drift Correction Control

        // Enables/disables magnetometer based yaw drift correction. Must also have mag calibration
        // data for this correction to work.
        void SetYawCorrectionEnabled(bool enable) { EnableYawCorrection = enable; }

        // Determines if yaw correction is enabled.
        bool IsYawCorrectionEnabled() const { return EnableYawCorrection; }

#if defined( ANDROID )

        bool HasMagCalibration() const { return true; }

#elif !defined( FBX_TOOL )
        // this is Android only implementation for now
        bool 		HasMagCalibration() const				{ OVR_COMPILER_ASSERT(false); return false; }
#endif

        // Sets the focus filter direction to the current HMD direction
        void SetFocusDirection();

        // Sets the focus filter to a direction in the body frame. Once set, a complementary filter
        // will very slowly drag the world to keep the direction of the HMD within the FOV of the focus
        void SetFocusDirection(Vector3f direction);

        // Sets the FOV (in radians) of the focus. When the yaw difference between the HMD's current pose
        // and the focus is smaller than the FOV, the complementary filter does not act.
        void SetFocusFOV(float rads);

        // Turns off the focus filter (equivalent to setting the focus to 0
        void ClearFocus();

        // *** Message Handler Logic

        // Notifies SensorFusion object about a new BodyFrame message from a sensor.
        // Should be called by user if not attaching to a sensor.
        void OnMessage(const MessageBodyFrame &msg) {
            //   OVR_ASSERT(!IsAttachedToSensor());
                HandleMessage(msg);
        }


    private:

        // Internal handler for messages; bypasses error checking.
        void HandleMessage(const MessageBodyFrame &msg);

        // Apply headset yaw correction from magnetometer
        // for models without camera or when camera isn't available
        void applyMagCalibratedYawCorrection(const Vector3f &magCalibrated, const Vector3f &gyro,
                                             float deltaT);

        void applyMagYawCorrection(Vector3f mag, Vector3f gyro, float deltaT);

        // Apply headset tilt correction from the accelerometer
        void applyTiltCorrection(float deltaT);

        // Apply camera focus correction
        void applyFocusCorrection(float deltaT);

//    class BodyFrameHandler : public MessageHandler
//    {
//        OcFusion* pFusion;
//    public:
//        BodyFrameHandler(OcFusion* fusion) : pFusion(fusion) { }
//        ~BodyFrameHandler();

//        virtual void OnMessage(const Message& msg);
//        virtual bool SupportsMessageType(MessageType type) const;
//    };   

        // This is the state needed by GetPredictionForTime()
        class StateForPrediction {
        public:
            // time the current state is correct for
            PoseStatef State;
            float Temperature;

            StateForPrediction() : Temperature(0) {};
        };

        struct MagReferencePoint {
            Vector3f MagUncalibratedInImuFrame;
            Quatf WorldFromImu;
            int Score;

            MagReferencePoint() {}

            MagReferencePoint(const Vector3f &magUncalibInImuFrame, const Quatf &worldFromImu,
                              int score)
                    : MagUncalibratedInImuFrame(magUncalibInImuFrame), WorldFromImu(worldFromImu),
                      Score(score) {}
        };


        bool getBufferedOrientation(Quatf *orientation, const Vector3f &gyro, float gyroThreshold,
                                    float deltaT);

//    BodyFrameHandler*		pHandler;

        // This can be read without any locks, so a high priority rendering thread doesn't
        // have to worry about being blocked by a sensor thread that got preempted.
        LocklessUpdater<StateForPrediction> UpdatedState;

        // The phase of the head as estimated by sensor fusion
        PoseStatef State;
        unsigned int Stage;

        SensorFilterBodyFrame FAccelHeadset;
        SensorFilterf FAngV;

        bool MotionTrackingEnabled;
        bool EnableGravity;
        bool EnableYawCorrection;
        bool GeomagneticSwitch;

        Array<MagReferencePoint> MagRefs;
        int MagRefIdx;
        float MagCorrectionIntegralTerm;

        // Apply compensation since magnetometer latency is higher than HMT sensors. We also don't perform mag yaw correction if the
        // angular velocity is too high.
        struct MagCompEntry {
            Quatf Orientation;
            float GyroMagnitude;
        };
        MagCompEntry MagLatencyCompBuffer[MagLatencyBufferSizeMax];
        int MagLatencyCompBufferIndex;
        int MagLatencyCompFillCount;
        float YawCorrectionTimer;

        Vector3f FocusDirection;
        float FocusFOV;

        Mutex RecenterMutex;
        Posef RecenterTransform;    // this is an additional transform that is applied to "recenter" the orientation in yaw.
//	LocklessUpdater<Posef>	RecenterTransform;	// this is an additional transform that is applied to "recenter" the orientation in yaw.
    };


} // namespace OVR
#endif
