#include "LogUtils.h"

#include <unistd.h>			// for gettid()
#include <sys/syscall.h>	// for syscall()
#include <stdarg.h>
#include <string.h>
#include <assert.h>


#include "sys/system_properties.h"

#if defined(OVR_ENABLE_CAPTURE)
#include <OVR_Capture.h>
#endif

#define DEBUG

// GPU Timer queries cause instability on current
// Adreno drivers. Disable by default, but allow
// enabling via the local prefs file.
// TODO: Test this on new Qualcomm driver under Lollipop.
static bool AllowGpuTimerQueries = false;

void SetAllowGpuTimerQueries(bool allow)
{
//	LOG("SetAllowGpuTimerQueries( %d )", allow);
	AllowGpuTimerQueries = allow;
}

void SetCurrentThreadAffinityMask(int mask)
{
	int  syscallres;
	pid_t pid = gettid();
	syscallres = syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);
	if (syscallres)
	{
		int err = errno;
//		WARN("Error in the syscall setaffinity: mask=%d=0x%x err=%d=0x%x", mask, mask, err, err);
	}
}


void InitLogUtils()
{
	char logFlag[16] = { 0 };
	__system_property_get("logFlag", logFlag);

	char trueflg[16] = "true";
    logflag = (strcmp(logFlag, trueflg) == 0);
}
void LogWithTagWarn(int prio,const char * TAG, const char * tag,const char * fmt, ... )
{
    #ifdef DEBUG
	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(prio, tag, fmt, ap);
	va_end(ap);
    #else

	char trueflg[16] = "true";
	bool flag = false;
	char state[16] = {0};
	if(!logflag)
	{
		__system_property_get(TAG, state);
	}
	if (strcmp(state, trueflg) == 0 || logflag)
	{
		va_list ap;
		va_start(ap, fmt);
		__android_log_vprint(prio, tag, fmt, ap);
		va_end(ap);
	}
    #endif
}

void LogWithTag(int prio,const char * tag, const char * fmt, ... )
{
    #ifdef DEBUG
	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(prio, tag, fmt, ap);
	va_end(ap);
    #else
	if( ANDROID_LOG_ERROR == prio )
	{
		va_list ap;
		va_start(ap, fmt);
		__android_log_vprint(prio, tag, fmt, ap);
		va_end(ap);
	}
    #endif
};

void LogWithFileTagWarn(int prio, const char * fileTag,const char * TAG,const char * fmt, ... )
{
    #ifdef DEBUG

	char strippedTag[128];
	const int len = strlen(fileTag);
	int slash;
	for (slash = len - 1; slash > 0 && fileTag[slash] != '/'; slash--) {
	}
	if (fileTag[slash] == '/') {
		slash++;
	}
	// copy forward until a dot or 0
	size_t i;
	for (i = 0; i < sizeof(strippedTag) - 1; i++) {
		const char c = fileTag[slash + i];
		if (c == '.' || c == 0) {
			break;
		}
		strippedTag[i] = c;
	}
	strippedTag[i] = '\0';

	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(prio, strippedTag, fmt, ap);
	va_end(ap);
    #else
	char trueflg[16] = "true";
	bool flag = false;

	char strippedTag[128];
	const int len = strlen(fileTag);
	int slash;
	for (slash = len - 1; slash > 0 && fileTag[slash] != '/'; slash--) {
	}
	if (fileTag[slash] == '/') {
		slash++;
	}
	// copy forward until a dot or 0
	size_t i;
	for (i = 0; i < sizeof(strippedTag) - 1; i++) {
		const char c = fileTag[slash + i];
		if (c == '.' || c == 0) {
			break;
		}
		strippedTag[i] = c;
	}
	strippedTag[i] = '\0';

	char state[16] = {0};
	char stateF[16] = {0};
	if(!logflag)
	{
		__system_property_get(TAG, state);
		char str[32] = {0};
		if(sizeof(strippedTag)>31)
		{
		    strncpy(str,strippedTag,31);
		    str[31] = '\0';
		}
		else
		{
			strcpy(str,strippedTag);
		}
		__system_property_get(str, stateF);
	}
	if (strcmp(state, trueflg) == 0 ||strcmp(stateF, trueflg) == 0|| logflag)
	{
		va_list ap;
		va_start(ap, fmt);
		__android_log_vprint(prio, strippedTag, fmt, ap);
		va_end(ap);
	}
    #endif
}

void LogWithFileTag(int prio, const char * fileTag,  const char * fmt, ... )
{
    #ifdef DEBUG
	char strippedTag[128];
	const int len = strlen(fileTag);
	int slash;
	for (slash = len - 1; slash > 0 && fileTag[slash] != '/'; slash--)
	{
	}
	if (fileTag[slash] == '/')
	{
		slash++;
	}
	// copy forward until a dot or 0
	size_t i;
	for (i = 0; i < sizeof(strippedTag) - 1; i++)
	{
		const char c = fileTag[slash + i];
		if (c == '.' || c == 0)
		{
			break;
		}
		strippedTag[i] = c;
	}
	strippedTag[i] = '\0';
	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(prio, strippedTag, fmt, ap);
	va_end(ap);

    #else
	if( ANDROID_LOG_ERROR == prio )
	{
		char strippedTag[128];
		const int len = strlen(fileTag);
		int slash;
		for (slash = len - 1; slash > 0 && fileTag[slash] != '/'; slash--)
		{
		}
		if (fileTag[slash] == '/')
		{
			slash++;
		}
		// copy forward until a dot or 0
		size_t i;
		for (i = 0; i < sizeof(strippedTag) - 1; i++)
		{
			const char c = fileTag[slash + i];
			if (c == '.' || c == 0) {
				break;
			}
			strippedTag[i] = c;
		}
		strippedTag[i] = '\0';
		va_list ap;
		va_start(ap, fmt);
		__android_log_vprint(prio, strippedTag, fmt, ap);
		va_end(ap);

	}
    #endif
};





