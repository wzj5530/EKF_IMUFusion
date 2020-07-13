#include <jni.h>
#include <string>
#include "NativeApp.h"
extern "C"
JNIEXPORT jlong JNICALL
Java_com_sensor_hello_hellosensor_HelloSenSorActivity_createNativeApp(
        JNIEnv* env,
        jobject jAct) {
    NativeApp *app = new NativeApp(jAct);
    app->StartNativeThread(app);
    return (jlong) app;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sensor_hello_hellosensor_HelloSenSorActivity_nativePause(
        JNIEnv* env,
        jobject /* this */,
        jlong appPtr) {
    ((NativeApp *)appPtr)->GetMessageQueue().SendPrintf( MsType_Pause, "pause " );
    ((NativeApp *)appPtr)->Native_SensorPause();
}

//Native nativeResume
extern "C"
JNIEXPORT void JNICALL
Java_com_sensor_hello_hellosensor_HelloSenSorActivity_nativeResume(
        JNIEnv* env,
        jobject /* this */,
        jlong appPtr) {
    ((NativeApp *)appPtr)->GetMessageQueue().SendPrintf(MsType_Resume, "resume " );
    ((NativeApp *)appPtr)->Native_SensorResume();
}

//Native nativeDestroy
extern "C"
JNIEXPORT void JNICALL
Java_com_sensor_hello_hellosensor_HelloSenSorActivity_nativeDestroy(
        JNIEnv* env,
        jobject /* this */,
        jlong appPtr) {

    ((NativeApp *)appPtr)->GetMessageQueue().SendPrintf( MsType_Destroy,"quit " );

}

extern "C"
JNIEXPORT void JNICALL
Java_com_sensor_hello_hellosensor_HelloSenSorActivity_nativeSurfaceCreated(
        JNIEnv* env,
        jobject /* this */,
        jlong appPtr) {
    ((NativeApp *)appPtr)->GetMessageQueue().SendPrintf( MsType_SurfaceView_Created, "surfaceCreated " );
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sensor_hello_hellosensor_HelloSenSorActivity_nativeSurfaceChanged(
        JNIEnv* env,
        jobject /* this */,
        jlong appPtr) {
    ((NativeApp *)appPtr)->GetMessageQueue().SendPrintf( MsType_SurfaceView_Changed, "surfaceChanged " );

}

extern "C"
JNIEXPORT void JNICALL
Java_com_sensor_hello_hellosensor_HelloSenSorActivity_nativeSurfaceDestroyed(
        JNIEnv* env,
        jobject /* this */,
        jlong appPtr) {
    ((NativeApp *)appPtr)->GetMessageQueue().PostPrintf( MsType_SurfaceView_Destroyed, "surfaceDestroyed " );

}