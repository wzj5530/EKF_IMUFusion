#include "GlProgram.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "GlUtils.h"
#include "LogUtils.h"


// Returns false and logs the ShaderInfoLog on failure.
static bool CompileShader( const GLuint shader, const char * src )
{
	glShaderSource( shader, 1, &src, 0 );
	glCompileShader( shader );

	GLint r;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &r );
	if ( r == GL_FALSE )
	{
//		LOG( "Compiling shader:\n%s\n****** failed ******\n", src );
		GLchar msg[4096];
		glGetShaderInfoLog( shader, sizeof( msg ), 0, msg );
//		LOG( "%s\n", msg );
		return false;
	}
	return true;
}

void GlProgram_Create( GlProgram * prog, const char * vertexSrc, const char * fragmentSrc )
{
	prog->vertexShader = glCreateShader( GL_VERTEX_SHADER );
	if ( !CompileShader( prog->vertexShader, vertexSrc ) )
	{
		LOG( "WZJGL Failed to compile vertex shader" );
	}
    else
    {
        LOG( "WZJGL Succes to compile vertex shader" );
    }
	prog->fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
	if ( !CompileShader( prog->fragmentShader, fragmentSrc ) )
	{
		LOG( "WZJGL Failed to compile fragment shader" );
	}
    else
    {
        LOG( "WZJGL Succes to compile fragment shader" );
    }
	prog->program = glCreateProgram();
	glAttachShader( prog->program, prog->vertexShader );
	glAttachShader( prog->program, prog->fragmentShader );

	// set attributes before linking
//	glBindAttribLocation( prog->program, VERTEX_ATTRIBUTE_LOCATION_POSITION,		"Position" );
//  glBindAttribLocation( prog->program, VERTEX_ATTRIBUTE_LOCATION_COLOR,			"Color" );
//	glBindAttribLocation( prog->program, VERTEX_ATTRIBUTE_LOCATION_NORMAL,			"Normal" );

	// link and error check
	glLinkProgram( prog->program );
	GLint r;
	glGetProgramiv( prog->program, GL_LINK_STATUS, &r );
	if ( r == GL_FALSE )
	{
		GLchar msg[1024];
		glGetProgramInfoLog( prog->program, sizeof( msg ), 0, msg );
		LOG( "WZJGL Linking program failed: %s\n", msg );
	}else
    {
        LOG( "WZJGL Linking program Succes" );
    }
    int error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        LOG("WZJGL: glError %d",error);
    }
//    glUseProgram( prog->program );
	prog->uM = glGetUniformLocation( prog->program, "u_Model" );
	prog->uMv = glGetUniformLocation( prog->program, "u_MVMatrix" );
    prog->uMvp = glGetUniformLocation( prog->program, "u_MVP" );
	if(prog->uMvp > 0)
	{
		LOG( "WZJGL Linking program uMvp uMvpuMvpuMvpuMvpuMvp sssss" );
	}
	else
	{
		LOG( "WZJGL Linking program uMvp uMvpuMvpuMvpuMvpuMvp fffff" );
	}
	prog->uLightPos = glGetUniformLocation( prog->program, "u_LightPos" );



//	// texture and image_external bindings
//	for ( int i = 0; i < 8; i++ )
//	{
//		char name[32];
//		sprintf( name, "Texture%i", i );
//		const GLint uTex = glGetUniformLocation( prog->program, name );
//		if ( uTex != -1 )
//		{
//			glUniform1i( uTex, i );
//		}
//	}

	glUseProgram( 0 );
}

void GlProgram_Destroy( GlProgram * prog )
{
	if ( prog->program != 0 )
	{
		glDeleteProgram( prog->program );
	}
	if ( prog->vertexShader != 0 )
	{
		glDeleteShader( prog->vertexShader );
	}
	if ( prog->fragmentShader != 0 )
	{
		glDeleteShader( prog->fragmentShader );
	}
	prog->program = 0;
	prog->vertexShader = 0;
	prog->fragmentShader = 0;
}

