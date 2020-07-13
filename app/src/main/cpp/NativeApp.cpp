#include <unistd.h>			// for usleep
#include <android/log.h>
#include <stdlib.h>
#include <stdio.h>
#include "NativeApp.h"
#include "LogUtils.h"
#include "IMU_Oculus/OVR_System.h"

typedef enum TrackerType {
    TRACKER_TYPE_IMU_CARDBOARD = 0,
    TRACKER_TYPE_IMU_OCULUS = 1,
    //TODO:: SLAM code to be added
    TRACKER_TYPE_SLAM_ORB = 2,
    TRACKER_TYPE_SLAM_VINS = 3

};

static const TrackerType currentTrackerType = TRACKER_TYPE_IMU_CARDBOARD;


JavaVM * VrLibJavaVM;
bool Native_OnLoad( JavaVM * JavaVm_ )
{
    LOG( "Native_OnLoad()" );

    if ( JavaVm_ == NULL )
    {
        FAIL( "JavaVm == NULL" );
    }
    if ( VrLibJavaVM != NULL )
    {
        // Should we silently return instead?
        LOG( "Native_OnLoad() called , return false" );
        return false;
    }

    VrLibJavaVM = JavaVm_;
    JNIEnv * jni;
    if ( JNI_OK != VrLibJavaVM->GetEnv( reinterpret_cast<void**>(&jni), JNI_VERSION_1_6 ) )
    {
        LOG( "Creating temporary JNIEnv" );
        const jint rtn = VrLibJavaVM->AttachCurrentThread( &jni, 0 );
        if ( rtn != JNI_OK )
        {
            FAIL( "AttachCurrentThread returned %i", rtn );
        }
    }
    else
    {
        LOG( "Using caller's JNIEnv" );
    }


    return true;
}
JNIEXPORT jint JNICALL JNI_OnLoad( JavaVM * vm, void * reserved ) {

    if (Native_OnLoad(vm)) {
        if (TRACKER_TYPE_IMU_OCULUS == currentTrackerType) {
            if (!OVR::System::IsInitialized()) {
                OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
            }
        }
    }
    return JNI_VERSION_1_6;
}
EGLDisplay m_display;
EGLSurface m_surface;
EGLContext m_context;
GLint window_width;;
GLint window_height;

#include <android/native_window_jni.h> // requires ndk r5 or newer
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>

jobject GetViewSurface(JNIEnv * jniEnv, jobject activity)
{
    jclass activityClass = jniEnv->GetObjectClass(activity);
    if (activityClass == NULL)
    {
        return NULL;
    }

    jfieldID svid = jniEnv->GetFieldID(activityClass, "MySurfaceView",
            "Landroid/view/SurfaceView;");
    if (svid != NULL)
    {
        jobject svObj = jniEnv->GetObjectField(activity, svid);
		if(svObj != NULL)
		{
		    jclass surfaceViewClass = jniEnv->GetObjectClass(svObj);
		    jmethodID mid = jniEnv->GetMethodID(surfaceViewClass, "getHolder", "()Landroid/view/SurfaceHolder;");
			if (mid == NULL)
			{
				return NULL;
			}
			jobject surfaceHolderObj = jniEnv->CallObjectMethod( svObj, mid);
			if (surfaceHolderObj == NULL)
			{
				return NULL;
			}
	        jclass surfaceHolderClass = jniEnv->GetObjectClass(surfaceHolderObj);
	        mid = jniEnv->GetMethodID(surfaceHolderClass, "getSurface", "()Landroid/view/Surface;");
	        if (mid == NULL)
	        {
	            return NULL;
	        }
	        return jniEnv->CallObjectMethod( surfaceHolderObj, mid);
		}
    }
    return NULL;
}

void surfacedestroy() {

	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_display, m_context);
    eglTerminate(m_display);

    m_display = EGL_NO_DISPLAY;
    m_surface = EGL_NO_SURFACE;
    m_context = EGL_NO_CONTEXT;
}
EGLint GetContextRenderableType ( EGLDisplay eglDisplay )
{
#ifdef EGL_KHR_create_context
    const char *extensions = eglQueryString ( eglDisplay, EGL_EXTENSIONS );

    // check whether EGL_KHR_create_context is in the extension string
    if ( extensions != NULL && strstr( extensions, "EGL_KHR_create_context" ) )
    {
        // extension is supported
        return EGL_OPENGL_ES3_BIT_KHR;
    }
#endif
    // extension is not supported
    return EGL_OPENGL_ES2_BIT;
}

bool initialize(ANativeWindow* _window)
{
    EGLConfig config;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

    window_width = ANativeWindow_getWidth ( _window );
    window_height = ANativeWindow_getHeight ( _window );
    LOG("eglInitialize() window_size ANativeWindow: %d X %d", window_width, window_height);
    if ((m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOG("eglGetDisplay() returned error %d", eglGetError());
        return false;
    }

    if ( !eglInitialize ( m_display, &majorVersion, &minorVersion ) )
    {
        LOG("eglInitialize() returned error %d", eglGetError());
        return false;
    }

    {
        EGLint numConfigs = 0;
        EGLint attribList[] =
        {
                EGL_RED_SIZE,       8,
                EGL_GREEN_SIZE,     8,
                EGL_BLUE_SIZE,      8,
                EGL_ALPHA_SIZE,     8,
                EGL_DEPTH_SIZE,     EGL_DONT_CARE,
                EGL_STENCIL_SIZE,   EGL_DONT_CARE,
                // if EGL_KHR_create_context extension is supported, then we will use
                // EGL_OPENGL_ES3_BIT_KHR instead of EGL_OPENGL_ES2_BIT in the attribute list
                EGL_RENDERABLE_TYPE, GetContextRenderableType ( m_display ),
                EGL_NONE
        };

        // Choose config
        if ( !eglChooseConfig ( m_display, attribList, &config, 1, &numConfigs ) )
        {
            LOG("eglChooseConfig() returned error %d", eglGetError());
            return GL_FALSE;
        }

        if ( numConfigs < 1 )
        {
            return GL_FALSE;
        }
    }

    {
        EGLint format = 0;
        eglGetConfigAttrib ( m_display, config, EGL_NATIVE_VISUAL_ID, &format );
        ANativeWindow_setBuffersGeometry ( _window, 0, 0, format );
    }

    const EGLint windowAttribs[] =
    {
            EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER,
            EGL_NONE
    };

    m_surface = eglCreateWindowSurface ( m_display, config,
                               _window, /*windowAttribs*/ NULL );

    if ( m_surface == EGL_NO_SURFACE )
    {
        return GL_FALSE;
    }
    GLint mVrViewerWidth;
    GLint mVrViewerHeight;
    eglQuerySurface( m_display, m_surface, EGL_WIDTH, &mVrViewerWidth );
    eglQuerySurface( m_display, m_surface, EGL_HEIGHT, &mVrViewerHeight );
    LOG("eglInitialize() window_size m_surface: %d X %d", window_width, window_height);

    m_context = eglCreateContext ( m_display, config,
                                               EGL_NO_CONTEXT, contextAttribs );

    if ( m_context == EGL_NO_CONTEXT )
    {
        LOG("eglCreateContext() returned error %d", eglGetError());
        return GL_FALSE;
    }

    // Make the context current
    if ( !eglMakeCurrent ( m_display, m_surface,
                           m_surface, m_context ) )
    {
        return GL_FALSE;
    }

    glDisable(GL_DITHER);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    return GL_TRUE;
}
NativeApp::NativeApp(jobject jAct):
		init( false ),
		SurfaceReady( false ),
        NativeMsQueue( 100 ),
		VrThreadPause( false )
{
	if ( VrLibJavaVM != NULL ){
		VrLibJavaVM->AttachCurrentThread(&Jni, 0);
	}
    if (jAct != 0) {
        jActivity = Jni->NewGlobalRef( jAct );
        Jni->DeleteLocalRef( jAct );
        jclass activityClass = Jni->GetObjectClass(jActivity);

    } else {
        LOG("jAct not set");
    }
    if(TRACKER_TYPE_IMU_OCULUS == currentTrackerType){
        phoneSensorInstance = OVR::PhoneSensor::GetInstance();
    } else if (TRACKER_TYPE_IMU_CARDBOARD == currentTrackerType){
        head_tracker_ = new cardboard::HeadTracker();
    }else if (TRACKER_TYPE_SLAM_ORB == currentTrackerType){
        //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
    }else if (TRACKER_TYPE_SLAM_VINS == currentTrackerType){
        //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
    }


    ReadyToExit= false;
}

NativeApp::~NativeApp()
{
    if(TRACKER_TYPE_IMU_OCULUS == currentTrackerType){
        delete phoneSensorInstance;
        phoneSensorInstance = NULL;
    } else if (TRACKER_TYPE_IMU_CARDBOARD == currentTrackerType){
        delete head_tracker_;
        head_tracker_ = NULL;
    }else if (TRACKER_TYPE_SLAM_ORB == currentTrackerType){
        //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
    }else if (TRACKER_TYPE_SLAM_VINS == currentTrackerType){
        //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
    }
}
void NativeApp::StartNativeThread(void *param)
{
    LOG( "StartNativeAppThread" );

    const int createErr = pthread_create( &NativeThread,
    		NULL /* default attributes */, &ThreadStarter, param );
    if ( createErr != 0 )
    {
        LOG( "Pthread_create returned %i", createErr );
    }
}

void* NativeApp::ThreadStarter( void * param )
{
    ((NativeApp *)param)->ThreadFunction();
    return NULL;
}


void NativeApp::Native_SensorPause() {
    if(TRACKER_TYPE_IMU_OCULUS == currentTrackerType){
        phoneSensorInstance->StopTrack();
    } else if (TRACKER_TYPE_IMU_CARDBOARD == currentTrackerType){
        head_tracker_->Pause();
    }else if (TRACKER_TYPE_SLAM_ORB == currentTrackerType){
        //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
    }else if (TRACKER_TYPE_SLAM_VINS == currentTrackerType){
        //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
    }
}

void NativeApp::Native_SensorResume(){
    if(TRACKER_TYPE_IMU_OCULUS == currentTrackerType){
        phoneSensorInstance->StartTrack();
    } else if (TRACKER_TYPE_IMU_CARDBOARD == currentTrackerType){
        head_tracker_->Resume();
    }else if (TRACKER_TYPE_SLAM_ORB == currentTrackerType){
        //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
    }else if (TRACKER_TYPE_SLAM_VINS == currentTrackerType){
        //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
    }
}

bool NativeApp::GL_CheckErrors( const char * logTitle )
{
	//TODO: Use the function in GlUtils
	bool hadError = false;
	// There can be multiple errors that need reporting.
	do
	{
		GLenum err = glGetError();
		if ( err == GL_NO_ERROR )
		{
			break;
		}
		hadError = true;
		LOG( "xxx %s GL Error: %d", logTitle, err  );
		if ( err == GL_OUT_OF_MEMORY )
		{
			LOG( "GL_OUT_OF_MEMORY" );
		}
	} while ( 1 );
	return hadError;
}

static constexpr uint64_t kNanosInSeconds = 1000000000;
constexpr uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

long GetMonotonicTimeNano() {
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return (res.tv_sec * kNanosInSeconds) + res.tv_nsec;
}
void NativeApp::ThreadFunction(){
	if ( VrLibJavaVM != NULL ){
		VrLibJavaVM->AttachCurrentThread(&Jni, 0);
		LOG( "NativeAppThread AttachCurrentThread" );
	}
	jclass activityClass = Jni->GetObjectClass(jActivity);
	if (activityClass != NULL)
	{
		javaClass = activityClass;
		LOG( "NativeAppThread GetObjectClass" );
	}
    pthread_setname_np(pthread_self(), "NativeAppThread");

    while( !ReadyToExit) {
        // Process incoming messages until queue is empty
    	for (;;) {
            Element *ele = NativeMsQueue.GetNextMessage();
            if (!ele) {
                break;
            }
            Command(ele);
            ele->clearMessageBody();
        }

        if(VrThreadPause&& (!ReadyToExit))
        {
            NativeMsQueue.SleepUntilMessage();
		    continue;
        }

        if (SurfaceReady) {

            std::array<float, 3> out_position;      //x,y,z
            std::array<float, 4> out_orientation;   //x,y,z,w
            long monotonic_time_nano = GetMonotonicTimeNano();
            monotonic_time_nano += kPredictionTimeWithoutVsyncNanos;
            if(TRACKER_TYPE_IMU_OCULUS == currentTrackerType){
                OVR::SensorState headPose = phoneSensorInstance->PredictedSensorState(double(monotonic_time_nano)* 0.000000001);
                out_position[0] = headPose.Predicted.Transform.Position.x;
                out_position[1] = headPose.Predicted.Transform.Position.y;
                out_position[2] = headPose.Predicted.Transform.Position.z;
                out_orientation[0] = -headPose.Predicted.Transform.Orientation.x;
                out_orientation[1] = -headPose.Predicted.Transform.Orientation.y;
                out_orientation[2] = -headPose.Predicted.Transform.Orientation.z;
                out_orientation[3] = headPose.Predicted.Transform.Orientation.w;
            } else if (TRACKER_TYPE_IMU_CARDBOARD == currentTrackerType){
                head_tracker_->GetPose(monotonic_time_nano, out_position, out_orientation);
            }else if (TRACKER_TYPE_SLAM_ORB == currentTrackerType){
                //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
            }else if (TRACKER_TYPE_SLAM_VINS == currentTrackerType){
                //TODO:: //TODO:: //TODO:: //TODO:: //TODO::
            }

//            out_position[0] = 0.0f;
//            out_position[1] = 0.0f;
//            out_position[2] = 0.0f;
//            out_orientation[0] = 0.0f;
//            out_orientation[1] = 0.0f;
//            out_orientation[2] = 0.0f;
//            out_orientation[3] = 1.0f;
            LOG("GetPose Position:%f,%f,%f--Rotation:%f,%f,%f,%f",out_position[0],out_position[1],out_position[2],
                out_orientation[0], out_orientation[1], out_orientation[2],out_orientation[3]);

			glClearColor(0.125, 0.125, 0.928, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
            DrawModel(floorGeometry,floorProg,out_orientation[0], out_orientation[1],out_orientation[2],out_orientation[3],
                      out_position[0],out_position[1],out_position[2], 0.0f, -20.0f, 0.0f,window_width,window_height);
            LOG("WZJGL DrawModel 6666");
            DrawModel(cubeGeometry,cubeProg,out_orientation[0], out_orientation[1],out_orientation[2],out_orientation[3],
                      out_position[0],out_position[1],out_position[2], 0.0f, 0.0f, -10.0f,window_width,window_height);

            DrawModel(cubeGeometry,cubeProg,out_orientation[0], out_orientation[1],out_orientation[2],out_orientation[3],
                      out_position[0],out_position[1],out_position[2], 5.0f, 0.0f, -10.0f,window_width,window_height);

            DrawModel(cubeGeometry,cubeProg,out_orientation[0], out_orientation[1],out_orientation[2],out_orientation[3],
                      out_position[0],out_position[1],out_position[2], -5.0f, 0.0f, -10.0f,window_width,window_height);

            DrawModel(cubeGeometry,cubeProg,out_orientation[0], out_orientation[1],out_orientation[2],out_orientation[3],
                      out_position[0],out_position[1],out_position[2], 0.0f, 5.0f, -10.0f,window_width,window_height);

            eglSwapBuffers(
                    eglGetDisplay( EGL_DEFAULT_DISPLAY ),
                    eglGetCurrentSurface( EGL_DRAW ));
            LOG("eglSwapBuffers called, EGL_DEFAULT_DISPLAY, EGL_DRAW");

            //TODO: Update frame rate control
			timespec	t, rem;
			t.tv_sec = 0;
			t.tv_nsec = 12 * 1e6;
			nanosleep( &t, &rem );
        }

    }
	if ( VrLibJavaVM != NULL ){
		VrLibJavaVM->DetachCurrentThread();
	}
}

void NativeApp::StopNativeThread()
{
    //NativeMsQueue.SendPrintf( MsType_Destroy,"quit " );
    const int ret = pthread_join( NativeThread, NULL );
    if ( ret != 0 )
    {
        LOG( "Failed to join NativeThread (%i)", ret );
    }
}

MessageQueue & NativeApp::GetMessageQueue()
{
    return NativeMsQueue;
}

void NativeApp::Command( Element * ele )
{


    if ( MsType_Pause == ele->getMessageType() )
    {
        LOG( "MessageType: %s", "MsType_Pause");
        VrThreadPause=true;
    }

    if ( MsType_Resume == ele->getMessageType() )
    {
        LOG( "MessageType: %s", "MsType_Resume");
        VrThreadPause=false;
    }

    if ( MsType_Destroy == ele->getMessageType() )
    {
        LOG( "MessageType: %s", "MsType_Destroy");

        ReadyToExit = true;
        surfacedestroy();

    }

    if ( MsType_SurfaceView_Created == ele->getMessageType() )
    {

    	GL_CheckErrors("SurfaceView  OnCreate 1");
        LOG( "MessageType: %s", "MsType_SurfaceView_Created");
        LOG( "MessageArgs: %s", ele->getMessageArgs());

        jobject surfaceObj = GetViewSurface(Jni, jActivity);
        GL_CheckErrors("SurfaceView  OnCreate 2");
        LOG( "Try, surfaceObj = %p ",surfaceObj);
    	ANativeWindow* _window;
    	if (surfaceObj != NULL) {
    		_window = ANativeWindow_fromSurface(Jni, surfaceObj);
    		LOG( "Try, surfaceObj = %p, nativeWin = %p",
    				surfaceObj, _window);
    		GL_CheckErrors("SurfaceView  OnCreate 3");
    		initialize(_window);
            GLSetUp();
        }
    	GL_CheckErrors("SurfaceView  OnCreate 4");
        GL_CheckErrors("SurfaceView  OnCreate 5");
        SurfaceReady = true;
    }

    if ( MsType_SurfaceView_Changed == ele->getMessageType() )
    {
    }

    if ( MsType_SurfaceView_Destroyed == ele->getMessageType() )
    {
        LOG( "MessageType: %s", "MsType_SurfaceView_Destroyed");
    }

    if ( MsType_Touch_Event == ele->getMessageType() )
    {
        LOG( "MessageType: %s", "MsType_Touch_Event");
    }

    if (100 <= ele->getMessageType())
    {
    	LOG( "in MessageType: UER_EVENT  %d", ele->getMessageType());
    }

}

void NativeApp::GLSetUp( )
{
    floorGeometry = GlGeometry_CreateFloor();
    GlProgram_Create(&floorProg, vertexShaderFloor, fragmentShaderFloor);

    cubeGeometry = GlGeometry_CreateCube();
    GlProgram_Create(&cubeProg, vertexShaderCube, fragmentShaderCube );
}



