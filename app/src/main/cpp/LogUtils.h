#if !defined(VRLib_Log_h)
#define VRLib_Log_h

#ifdef _WINDOWS_		// allow this file to be included in PC projects
#define LOG( ... ) printf( __VA_ARGS__ );printf("\n")
#else

#include <android/log.h>
#include <stdlib.h>

#include <stdio.h>


static  bool logflag ;

void InitLogUtils();
void LogWithTag(int prio, const char * tag, const char * fmt, ...);
void LogWithTagWarn(int prio, const char * TAG, const char * tag, const char * fmt, ...);

void LogWithFileTag(int prio, const char * fileTag, const char * fmt, ...);
void LogWithFileTagWarn(int prio, const char * fileTag, const char * TAG, const char * fmt, ...);

#define LOG(...) LOGIF(__VA_ARGS__)
#define LOGIF( ... ) LogWithFileTag( ANDROID_LOG_INFO, __FILE__,__VA_ARGS__ )
#define LOGWF(__TAG__, ... ) LogWithFileTagWarn(ANDROID_LOG_WARN,__FILE__,__TAG__, __VA_ARGS__ )
#define LOGEF( ... ) LogWithFileTag(ANDROID_LOG_ERROR,__FILE__,__VA_ARGS__ )

#define FAIL( ... ) {LOGEF(__VA_ARGS__ );abort();}

#define LOGI(__tag__, ... ) LogWithTag( ANDROID_LOG_INFO, __tag__, __VA_ARGS__ )
#define LOGW(__TAG__,__tag__, ... ) LogWithTagWarn(ANDROID_LOG_WARN,__TAG__,__tag__, __VA_ARGS__ )
#define LOGE(__tag__, ... ) LogWithTag(ANDROID_LOG_ERROR, __tag__, __VA_ARGS__ )


// LOG (usually defined on a per-file basis to write to a specific tag) is for logging that can be checked in 
// enabled and generally only prints once or infrequently.
// SPAM is for logging you want to see every frame but should never be checked in
#if defined( OVR_BUILD_DEBUG )
// you should always comment this out before checkin
//#define ALLOW_LOG_SPAM
#endif

#if defined( ALLOW_LOG_SPAM )
#define SPAM(...) LogWithTag( ANDROID_LOG_VERBOSE, "Spam", __VA_ARGS__ )
#else
#define SPAM(...) { }
#endif

// TODO: we need a define for internal builds that will compile in assertion messages but not debug breaks
// and we need a define for external builds that will do nothing when an assert is hit.
#if !defined( OVR_BUILD_DEBUG )
#define DROID_ASSERT( __expr__, __tag__ ) { if ( !( __expr__ ) ) { DROIDWARN( __tag__, "ASSERTION FAILED: %s", #__expr__ ); } }
#else
#define DROID_ASSERT( __expr__, __tag__ ) { if ( !( __expr__ ) ) { DROIDWARN( __tag__, "ASSERTION FAILED: %s", #__expr__ ); OVR_DEBUG_BREAK; } }
#endif


#include <time.h>

// Declaring a variable with this class will report the time elapsed when it
// goes out of scope.
class LogCpuTime
{
public:

	LogCpuTime(const char * label) :
		Label(label)
	{
		StartTimeNanoSec = GetNanoSeconds();
	}
	~LogCpuTime()
	{
		const double endTimeNanoSec = GetNanoSeconds();
//		DROIDLOG("LogCpuTime", "%s took %6.4f seconds", Label, (endTimeNanoSec - StartTimeNanoSec) * 1e-9);
	}

	static double GetNanoSeconds()
	{
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return (double)now.tv_sec * 1e9 + now.tv_nsec;
	}

private:
	const char *	Label;
	double			StartTimeNanoSec;
};

// Call LogGpuTime::Begin() and LogGpuTime::End() to log the GPU rendering time between begin and end.
// Note that begin-end blocks cannot overlap.
// This seems to cause some stability problems, so don't do it automatically.
template< int NumTimers, int NumFrames = 10 >
class LogGpuTime
{
public:
	LogGpuTime();
	~LogGpuTime();

	void			Begin(int index);
	void			End(int index);
	void			PrintTime(int index, const char * label) const;
	double			GetTime(int index) const;
	double			GetTotalTime() const;

private:
	bool			UseTimerQuery;
	bool			UseQueryCounter;
	uint32_t		TimerQuery[NumTimers];
	int64_t			BeginTimestamp[NumTimers];
	int32_t			DisjointOccurred[NumTimers];
	int32_t			TimeResultIndex[NumTimers];
	double			TimeResultMilliseconds[NumTimers][NumFrames];
	int				LastIndex;
};

// Allow GPU Timer Queries - NOTE: GPU Timer queries
// can cause instability on current Adreno driver.
void SetAllowGpuTimerQueries(bool allow);

// For CPU performance testing ONLY!
void SetCurrentThreadAffinityMask(int mask);

#endif	// ! _WINDOWS_

#endif // VRLib_Log_h
