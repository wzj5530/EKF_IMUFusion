#ifndef GlUtils_h
#define GlUtils_h

// Everyone else should include GlUtils.h to get the GL headers instead of
// individually including them, so if we have to change what we include,
// we can do it in one place.
#define __gl2_h_


#include <EGL/egl.h>
#include <EGL/eglext.h>
#ifdef __gl2_h_

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
static const int GL_ES_VERSION = 3;	// This will be passed to EglSetup() by App.cpp

#else

#include <GLES2/gl2.h>
static const int GL_ES_VERSION = 2;	// This will be passed to EglSetup() by App.cpp

#endif // __gl2_h_

#include <GLES2/gl2ext.h>

#endif	// GlUtils_h
