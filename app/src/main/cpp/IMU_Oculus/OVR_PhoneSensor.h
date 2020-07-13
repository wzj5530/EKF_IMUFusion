#ifndef OVR_PHONE_SENSOR_H
#define OVR_PHONE_SENSOR_H

#include <errno.h>
#include <android/sensor.h>
#include <stdio.h>
#include "OVR_BaseSensor.h"
#include "OVR_DataStructure.h"
#include "OVR_Fusion.h"

namespace OVR {
    class PhoneSensor : public BaseSensor {
    public:
        static PhoneSensor *GetInstance() {
            if (!mpInstance)
                mpInstance = new PhoneSensor();
            return mpInstance;
        }

        virtual ~PhoneSensor();

        void StartTrack();

        void ResetTrack();

        void StopTrack();

        bool IsGyroscopeAvailable();

        static void *SensorThreadMain(void *arg);

        static int SensorCallback(int fd, int events, void *data);

        static void HandleAccelerometerEvent(ASensorEvent &sensorEvent);

        static void HandleGyroscopeEvent(ASensorEvent &sensorEvent);

        static void HandleMagEvent(ASensorEvent &sensorEvent);


        SensorState PredictedSensorState(double absTime);


    private:
        PhoneSensor();

        PhoneSensor(PhoneSensor const &);

        PhoneSensor &operator=(PhoneSensor const &);

        static PhoneSensor *mpInstance;

        ASensorRef mAccSensorRef;
        ASensorRef mGyroSensorRef;
        ASensorRef mMagSensorRef;
        ASensorManager *mpSensorManager;
        ASensorEventQueue *mpSensorEventQueue;

        pthread_t mSensorThread;
        bool mSensorThreadExit;
        ALooper *mpSensorThreadLooper;
        pthread_cond_t mSensorThreadReadyCv;
        pthread_mutex_t mSensorThreadReadyMutex;
        bool mSensorThreadReady;

        OcFusion *mpOcFusion;
        MessageBodyFrame mSensorData;
        SensorState mLastSensorState;
        float mTimeDeltaMs;
        int64_t mLastGyroTimestampNs;
        bool mSensorThreadRunning;
    };

} /* namespace OVR */
#endif /* PHONE_SENSOR_H_ */
