#ifndef OVR_SENSOR_TYPES_H
#define OVR_SENSOR_TYPES_H

#include "OVR_Math.h"

namespace OVR {

//-------------------------------------------------------------------------------------
// ***** Sensor State

// These values are reported as compatible with VrApi.


// PoseState describes the complete pose, or a rigid body configuration, at a
// point in time, including first and second derivatives. It is used to specify
// instantaneous location and movement of the headset.
// SensorState is returned as a part of the sensor state.
template<class T>
class PoseState
{
public:
    typedef typename CompatibleTypes<Pose<T> >::Type CompatibleType;

    PoseState() : TimeInSeconds(0.0) { }
    // float <-> double conversion constructor.
    explicit PoseState(const PoseState<typename Math<T>::OtherFloatType> &src)
        : Transform(src.Transform),
          AngularVelocity(src.AngularVelocity), LinearVelocity(src.LinearVelocity),
          AngularAcceleration(src.AngularAcceleration), LinearAcceleration(src.LinearAcceleration),
          TimeInSeconds(src.TimeInSeconds)
    { }

    // C-interop support: PoseStatef <-> ovrPoseStatef
    PoseState(const typename CompatibleTypes<PoseState<T> >::Type& src)
        : Transform(src.Pose),
          AngularVelocity(src.AngularVelocity), LinearVelocity(src.LinearVelocity),
          AngularAcceleration(src.AngularAcceleration), LinearAcceleration(src.LinearAcceleration),
          TimeInSeconds(src.TimeInSeconds)
    { }

    operator const typename CompatibleTypes<PoseState<T> >::Type () const
    {
        typename CompatibleTypes<PoseState<T> >::Type result;
        result.Pose		            = Transform;
        result.AngularVelocity      = AngularVelocity;
        result.LinearVelocity       = LinearVelocity;
        result.AngularAcceleration  = AngularAcceleration;
        result.LinearAcceleration   = LinearAcceleration;
        result.TimeInSeconds        = TimeInSeconds;
        return result;
    }


    Pose<T>     Transform;
    Vector3<T>  AngularVelocity;
    Vector3<T>  LinearVelocity;
    Vector3<T>  AngularAcceleration;
    Vector3<T>  LinearAcceleration;
    // Absolute time of this state sample; always a double measured in seconds.
    double      TimeInSeconds;
};

typedef PoseState<float>  PoseStatef;
// Bit flags describing the current status of sensor tracking.
enum StatusBits
{
    Status_OrientationTracked    = 0x0001,   // Orientation is currently tracked (connected and in use).
    Status_PositionTracked       = 0x0002,   // Position is currently tracked (false if out of range).
    Status_PositionConnected     = 0x0020,   // Position tracking HW is conceded.
    Status_HmdConnected          = 0x0080    // HMD Display is available & connected.
};



// Full state of of the sensor reported by GetSensorState() at a given absolute time.
class SensorState
{
public:
    SensorState() : Temperature(0), Status(0) { }

    // C-interop support
    SensorState(const ovrSensorState& s);
    operator const ovrSensorState& () const;

    // Pose state at the time that SensorState was requested.
    PoseStatef   Predicted;
    // Actual recorded pose configuration based on sensor sample at a
    // moment closest to the requested time.
    PoseStatef   Recorded;

    // Sensor temperature reading, in degrees Celcius, as sample time.
    float        Temperature;
    // Sensor status described by ovrStatusBits.
    unsigned     Status;
    PoseStatef   FixedSensor;
};

}
#endif/*SENSOR_TYPES_H*/
