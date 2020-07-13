#ifndef HELLO_NATIVEAPP_H
#define HELLO_NATIVEAPP_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <jni.h>
#include <pthread.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "MessageQueue.h"
#include "GlProgram.h"
#include "GlGeometry.h"
#include "GlUtils.h"
#include "IMU_CardBoard/head_tracker.h"
#include "IMU_Oculus/OVR_PhoneSensor.h"

class NativeApp {
public:
    NativeApp(jobject jAct);
    ~NativeApp();

    // variables used by OC messageQ
    bool            init;
    bool            SurfaceReady;
    pthread_t		NativeThread;
    MessageQueue	NativeMsQueue;
    bool            ReadyToExit;
    bool            VrThreadPause;
	
	JNIEnv *		Jni;
	jobject			jActivity;	//Global reference for Activity object
	jclass			javaClass;

	GlProgram   floorProg;
	GlProgram   cubeProg;
	GlGeometry  floorGeometry;
	GlGeometry  cubeGeometry;

	cardboard::HeadTracker* head_tracker_ = NULL;
	OVR::PhoneSensor* phoneSensorInstance = NULL;


    //Methods used by OC messageQ
    static void *       ThreadStarter(void * param);
    MessageQueue &       GetMessageQueue();
    void 	            ThreadFunction();
    void                StartNativeThread(void *param);
    void                StopNativeThread();
    void                Command(Element *msg);

    void 				Native_SensorPause();
    void 				Native_SensorResume();

    void 				GLSetUp();
    bool 				GL_CheckErrors( const char * logTitle );
};
#endif //HELLO_NATIVEAPP_H
