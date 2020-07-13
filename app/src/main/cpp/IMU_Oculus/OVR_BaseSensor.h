#ifndef OVR_BASE_SENSOR_H
#define OVR_BASE_SENSOR_H
namespace OVR {
    class BaseSensor {
    public:
        virtual void StartTrack() = 0;

        virtual void ResetTrack() = 0;

        virtual void StopTrack() = 0;
    };
}
#endif /* BASE_SENSOR_H */
