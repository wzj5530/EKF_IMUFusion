#ifndef GlGeometry_h
#define GlGeometry_h

#include "GlUtils.h"
#include "GlProgram.h"

struct GlGeometry
{
	GlGeometry() :
					vertexBuffer( 0 ),
					indexBuffer( 0 ),
					vertexArrayObject( 0 ),
					vertexCount( 0 ),
					indexCount( 0 ) {}

	GLuint		vertexBuffer;
	GLuint		indexBuffer;
	GLuint	 	vertexArrayObject;
	int			vertexCount;
	int 		indexCount;
};


GlGeometry GlGeometry_CreateFloor();
GlGeometry GlGeometry_CreateCube();
void DrawModel(GlGeometry& glMesh, GlProgram& glProg,float x, float y, float z, float w, float px, float py, float pz,
			   float mx, float my, float mz, int window_width, int window_height);


#endif	// GlGeometry_h
