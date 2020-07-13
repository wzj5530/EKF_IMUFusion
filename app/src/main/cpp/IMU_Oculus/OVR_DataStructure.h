#ifndef OVR_DATA_STRUCTURE_H
#define OVR_DATA_STRUCTURE_H

#include "OVR_Math.h"

namespace OVR {

    struct TrackerSample {
        SInt32 AccelX, AccelY, AccelZ;
        SInt32 GyroX, GyroY, GyroZ;
    };

    struct TrackerSensors {
        UByte SampleCount;
        UInt16 Timestamp;
        UInt16 LastCommandID;
        SInt16 Temperature;
        TrackerSample Samples[3];
        SInt16 MagX, MagY, MagZ;
    };



// Sensor BodyFrame notification.
// Sensor uses Right-Handed coordinate system to return results, with the following
// axis definitions:
//  - Y Up positive
//  - X Right Positive
//  - Z Back Positive
// Rotations a counter-clockwise (CCW) while looking in the negative direction
// of the axis. This means they are interpreted as follows:
//  - Roll is rotation around Z, counter-clockwise (tilting left) in XY plane.
//  - Yaw is rotation around Y, positive for turning left.
//  - Pitch is rotation around X, positive for pitching up.

    struct MessageBodyFrame {
        Vector3f Acceleration;  // Acceleration in m/s^2.
        Vector3f RotationRate;  // Angular velocity in rad/s.
        Vector3f MagneticField; // Magnetic field strength in Gauss.
        Vector3f MagneticBias;  // Magnetic field calibration bias in Gauss.
        float Temperature;   // Temperature reading on sensor surface, in degrees Celsius.
        float TimeDelta;     // Time passed since last Body Frame, in seconds.

        // The absolute time from the host computers perspective that the message should be
        // interpreted as.  This is derived from the rolling 16 bit timestamp on the incoming
        // messages and a continuously correcting delta value maintained by SensorDeviceImpl.
        //
        // Integration should use TimeDelta, but prediction into the future should derive
        // the delta time from PredictToSeconds - AbsoluteTimeSeconds.
        //
        // This value will always be <= the return from a call to Timer::GetSeconds().
        //
        // This value will not usually be an integral number of milliseconds, and it will
        // drift by fractions of a millisecond as the time delta between the sensor and
        // host is continuously adjusted.
        double AbsoluteTimeSeconds;
    };
} // namespace OVR
#endif
