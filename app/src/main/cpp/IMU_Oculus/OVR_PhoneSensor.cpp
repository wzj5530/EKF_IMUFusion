#include "OVR_PhoneSensor.h"
#include "OVR_Timer.h"

#define SENSOR_THREAD_LOOPER_WAIT 250

char g_SdcardPath[1024];

namespace OVR {
    PhoneSensor *PhoneSensor::mpInstance = NULL;
    unsigned int StaticCount = 0;

    PhoneSensor::PhoneSensor()
    {
        mLastGyroTimestampNs = 0;
        mSensorThreadRunning = false;
        mpOcFusion = new OVR::OcFusion();

    }

    PhoneSensor::~PhoneSensor()
    {
        delete mpOcFusion;
        PhoneSensor::mpInstance = NULL;
    }


    void PhoneSensor::StartTrack()
    {
        LOGIF("PhoneSensor StartTrack begin");

        if (mSensorThreadRunning == false) {
            LOGIF("PhoneSensor StartTrack : create sensor thread");
        } else {
            LOGIF("PhoneSensor StartTrack : sensor thread is exists ,return");
            return;
        }

        //Get a reference to the sensor manager.
        mpSensorManager = ASensorManager_getInstance();

        //Start up the sensor looper thread
        pthread_mutex_init(&mSensorThreadReadyMutex, NULL);
        pthread_cond_init(&mSensorThreadReadyCv, NULL);
        mSensorThreadReady = false;
        mSensorThreadExit = false;

        int status = pthread_create(&mSensorThread, NULL, SensorThreadMain, NULL);
        if (status != 0) {
            LOGEF("StartTrack: Failed to create sensor thread");
        }
        pthread_setname_np(mSensorThread, "PhoneSensor");

        //Wait for the sensor thread to be setup and ready
        pthread_mutex_lock(&mSensorThreadReadyMutex);
        while (!mSensorThreadReady) {
            pthread_cond_wait(&mSensorThreadReadyCv, &mSensorThreadReadyMutex);
        }
        pthread_mutex_unlock(&mSensorThreadReadyMutex);

        mSensorThreadRunning = true;

        //Grab the default sensor for each type
        mAccSensorRef = ASensorManager_getDefaultSensor(mpSensorManager,
                                                        ASENSOR_TYPE_ACCELEROMETER);
        mGyroSensorRef = ASensorManager_getDefaultSensor(mpSensorManager,
                                                         ASENSOR_TYPE_GYROSCOPE);
        mMagSensorRef = ASensorManager_getDefaultSensor(mpSensorManager,
                                                        ASENSOR_TYPE_MAGNETIC_FIELD);

        //Creates a new sensor event queue and associate it with a looper.
        mpSensorEventQueue = ASensorManager_createEventQueue(mpSensorManager,
                                                             mpSensorThreadLooper, 0,
                                                             SensorCallback, NULL);
        LOGIF("Create Sensor EventQueue");

        if (mAccSensorRef != NULL) {
            LOGIF("Enabling acc sensor...");
            ASensorEventQueue_enableSensor(mpSensorEventQueue, mAccSensorRef);

            int minDelay = ASensor_getMinDelay(mAccSensorRef);
            LOGIF("Min Acceleration Sensor Delay : %0.2f ms", (float) minDelay / 1000.0f);

            ASensorEventQueue_setEventRate(mpSensorEventQueue, mAccSensorRef, minDelay);
        } else {
            LOGEF("Acceleration is NOT avaiable!");
        }

        if (mGyroSensorRef != NULL) {
            LOGIF("Enabling gyro sensor...");
            ASensorEventQueue_enableSensor(mpSensorEventQueue, mGyroSensorRef);
            int minDelay = ASensor_getMinDelay(mGyroSensorRef);
            LOGIF("Min Gyroscope Sensor Delay : %0.2f ms", (float) minDelay / 1000.0f);
            mTimeDeltaMs = minDelay / 1000.0f;
            ASensorEventQueue_setEventRate(mpSensorEventQueue,
                                           mGyroSensorRef, minDelay);
        } else {
            LOGEF("Gyroscope is NOT avaiable!");
        }

        if (mMagSensorRef != NULL) {
            LOG("Enabling mag sensor...");
            ASensorEventQueue_enableSensor(mpSensorEventQueue, mMagSensorRef);
            int minDelay = ASensor_getMinDelay(mMagSensorRef);
            LOG("Min Mag Sensor Delay : %0.2f ms", (float) minDelay / 1000.0f);
            mTimeDeltaMs = minDelay / 1000.0f;
            ASensorEventQueue_setEventRate(mpSensorEventQueue,
                                           mMagSensorRef, minDelay);
        } else {
            LOG("mag is NOT avaiable!");
        }

        LOGIF("PhoneSensor StartTrack end");
    }

    void PhoneSensor::StopTrack()
    {
        //Signal our sensor thread to stop
        LOGIF("StopTrack begin");
        if (mpSensorEventQueue == NULL) {
            LOGIF("stopTrack have been done, do not need again!!!");
            return;
        }
        mSensorThreadExit = true;

        //LOGIF("Waking the looper...");
        ALooper_wake(mpSensorThreadLooper);

        LOGIF("Waiting for the sensor thread to exit...");
        pthread_join(mSensorThread, NULL);

        mSensorThreadRunning = false;

        //Disable the sensors
        if (mAccSensorRef != NULL) {
            LOGIF("Disabling acc sensor...");
            ASensorEventQueue_disableSensor(mpSensorEventQueue,
                                            mAccSensorRef);
        }

        if (mGyroSensorRef != NULL) {
            LOGIF("Disabling gyro sensor...");
            ASensorEventQueue_disableSensor(mpSensorEventQueue,
                                            mGyroSensorRef);
        }

        if (mMagSensorRef != NULL) {
            LOGIF("Disabling mag sensor...");
            ASensorEventQueue_disableSensor(mpSensorEventQueue,
                                            mMagSensorRef);
        }

        //Destroy the event queue
        // THIS NEVER RETURNS!
        ////LOGIF("Destroying sensor event queue...");
        //ASensorManager_destroyEventQueue(sensorManager, sensorEventQueue);
        mpSensorEventQueue = NULL;
        //sensorThread = -1;
        //LOGIF("StopTrack end");
    }

    void PhoneSensor::ResetTrack()
    {
        LOGIF("Reset Tracker.");
        mLastGyroTimestampNs = 0;//To ensure the delatT correct when resum
        mpOcFusion->Reset();
    }

    void *PhoneSensor::SensorThreadMain(void *arg)
    {
        PhoneSensor *pSensorInstance = PhoneSensor::GetInstance();

        pSensorInstance->mpSensorThreadLooper = ALooper_prepare(0);

        //Signal the waiting thread that the looper has been created for this thread
        //and we're ready to receive events
        pthread_mutex_lock(&pSensorInstance->mSensorThreadReadyMutex);
        pSensorInstance->mSensorThreadReady = true;
        pthread_cond_signal(&pSensorInstance->mSensorThreadReadyCv);
        pthread_mutex_unlock(&pSensorInstance->mSensorThreadReadyMutex);

        //LOGIF("Sensor Thread starting loop");
        while (1) {
            int id;
            int events;
            // Poll the events, which causes the callback function to be invoked.
            // This routine basically never returns until ALooper_wake() is called.
            // When called this returns with ALOOPER_POLL_WAKE.
            id = ALooper_pollAll(SENSOR_THREAD_LOOPER_WAIT, NULL, &events, 0);
            switch (id) {
                case ALOOPER_POLL_WAKE:     // -1
                    // This should not be spam since only called when exiting
                    //LOGIF("ALooper_pollAll() returned ALOOPER_POLL_WAKE! %d Events", events);
                    break;

                case ALOOPER_POLL_CALLBACK: // -2
                    //LOGIF("ALooper_pollAll() returned ALOOPER_POLL_CALLBACK! %d Events", events);
                    break;

                case ALOOPER_POLL_TIMEOUT:  // -3
                    // Since this happens all the time we don't want to spam
                    ////LOGIF("ALooper_pollAll() returned ALOOPER_POLL_TIMEOUT! %d Events", events);
                    break;

                case ALOOPER_POLL_ERROR:    // -4
                    //LOGIF("ALooper_pollAll() returned ALOOPER_POLL_ERROR! %d Events", events);
                    break;

                default:
                    //LOGIF("ALooper_pollAll() returned Unknown (%d)! %d Events", id, events);
                    break;
            }

            //Check if the main thread has requested an exit
            if (pSensorInstance->mSensorThreadExit) {
                // There is a timing issue here.  If we leave too quickly then pthread_join() below
                // hangs forever.  Simply logging stuff to logcat is enough of a delay. Therefore,
                // add a wait here.
                timespec t, rem;
                t.tv_sec = 0;
                t.tv_nsec = 250000000;  // 250 million nanoseconds is 250 milliseconds
                nanosleep(&t, &rem);
                //LOGIF("Sensor Thread Exiting!");
                return 0;
            }
        }   // while (1)

    }

    int PhoneSensor::SensorCallback(int fd, int events, void *data)
    {
        PhoneSensor *pSensorInstance = PhoneSensor::GetInstance();

        ASensorEvent sensorEvent;

        //LOGIF("    SensorCallback");
        while (!pSensorInstance->mSensorThreadExit &&
               ASensorEventQueue_getEvents(pSensorInstance->mpSensorEventQueue,
                                           &sensorEvent, 1) > 0) {
            switch (sensorEvent.type) {
                case ASENSOR_TYPE_ACCELEROMETER:
                    //LOGIF("Acc event: x=%f, y=%f, z=%f", sensorEvent.acceleration.x,
                    //	sensorEvent.acceleration.y,sensorEvent.acceleration.z);
                    HandleAccelerometerEvent(sensorEvent);
                    break;
                case ASENSOR_TYPE_GYROSCOPE:
                    //LOGIF("Gyro event: x=%f, y=%f, z=%f", sensorEvent.vector.x,
                    //      sensorEvent.vector.y,sensorEvent.vector.z);
                    HandleGyroscopeEvent(sensorEvent);
                    break;

                case ASENSOR_TYPE_MAGNETIC_FIELD:
                    //LOGIF("Gyro event: x=%f, y=%f, z=%f", sensorEvent.vector.x,
                    //      sensorEvent.vector.y,sensorEvent.vector.z);
                    HandleMagEvent(sensorEvent);
                    break;

                case ASENSOR_TYPE_ACCELEROMETER + 100:
                    //LOGIF("Acc event: x=%f, y=%f, z=%f", sensorEvent.acceleration.x,
                    //	sensorEvent.acceleration.y,sensorEvent.acceleration.z);
                    HandleAccelerometerEvent(sensorEvent);
                    break;
                case ASENSOR_TYPE_GYROSCOPE + 100:
                    //LOGIF("Gyro event: x=%f, y=%f, z=%f", sensorEvent.vector.x,
                    //      sensorEvent.vector.y,sensorEvent.vector.z);
                    HandleGyroscopeEvent(sensorEvent);
                    break;
                default:;

            }
        }

        return 1;
    }

    void PhoneSensor::HandleAccelerometerEvent(ASensorEvent &sensorEvent)
    {
        PhoneSensor *pSensorInstance = PhoneSensor::GetInstance();
        pSensorInstance->mSensorData.Acceleration = OVR::Vector3f(-sensorEvent.acceleration.y,
                                                                  sensorEvent.acceleration.x,
                                                                  sensorEvent.acceleration.z);
    }

    void PhoneSensor::HandleGyroscopeEvent(ASensorEvent &sensorEvent)
    {
        PhoneSensor *pSensorInstance = PhoneSensor::GetInstance();
        pSensorInstance->mSensorData.RotationRate = OVR::Vector3f(-sensorEvent.vector.y,
                                                                  sensorEvent.vector.x,
                                                                  sensorEvent.vector.z);

        pSensorInstance->mSensorData.TimeDelta =
                float((sensorEvent.timestamp - pSensorInstance->mLastGyroTimestampNs) /
                      1000000000.0f);
        pSensorInstance->mSensorData.AbsoluteTimeSeconds = OVR::Timer::GetSeconds();

        if (0 != pSensorInstance->mLastGyroTimestampNs) {
            pSensorInstance->mpOcFusion->OnMessage(pSensorInstance->mSensorData);
        }
        pSensorInstance->mLastGyroTimestampNs = sensorEvent.timestamp;

    }

    void PhoneSensor::HandleMagEvent(ASensorEvent &sensorEvent) {
        PhoneSensor *pSensorInstance = PhoneSensor::GetInstance();
        pSensorInstance->mSensorData.MagneticField = OVR::Vector3f(sensorEvent.vector.x,
                                                                   sensorEvent.vector.y,
                                                                   sensorEvent.vector.z);
    }

    SensorState PhoneSensor::PredictedSensorState(double absTime)
    {
        mLastSensorState = mpOcFusion->GetPredictionForTime(absTime);
        return mLastSensorState;
    }

    bool PhoneSensor::IsGyroscopeAvailable() {
        ASensorManager *pSensorManager = ASensorManager_getInstance();
        ASensorRef gyroRef = ASensorManager_getDefaultSensor(pSensorManager,
                                                             ASENSOR_TYPE_GYROSCOPE);
        return (gyroRef != 0);
    }

}//namespace OVR
