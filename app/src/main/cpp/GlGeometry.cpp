#include "GlGeometry.h"
#include "GlProgram.h"
#include "LogUtils.h"

#define FLOOR_VERTEX_POS_SIZE      3     // px, py, pz
#define FLOOR_VERTEX_COLOR_SIZE    4     // r, g, b, a
#define FLOOR_VERTEX_NORMAL_SIZE   3     // nx, ny, nz

#define CUBE_VERTEX_POS_SIZE      3     // px, py, pz
#define CUBE_VERTEX_COLOR_SIZE    4     // r, g, b, a
#define CUBE_VERTEX_NORMAL_SIZE   3     // nx, ny, nz
static const float FloorPoints[]  = {
        // +X, +Z quadrant
        2000.0f, 0.0f, 0.0f,                //pos
        0.0f, 0.9023f, 0.3398f, 1.0f,       //color
        0.0f, 1.0f, 0.0f,                   //normal

        0.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        2000.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0, 0, 2000,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        2000.0f, 0.0f, 2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        // -X, +Z quadrant
        0.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        -2000.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        -2000.0f, 0.0f, 2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        -2000.0f, 0.0f, 2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        // +X, -Z quadrant
        2000.0f, 0.0f, -2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, -2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        2000.0f, 0.0f, -2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        2000.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        // -X, -Z quadrant
        0.0f, 0.0f, -2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        -2000.0f, 0.0f, -2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        -2000.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, -2000.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        -2000.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.0f, 0.9023f, 0.3398f, 1.0f,
        0.0f, 1.0f, 0.0f,
};

static const unsigned short FloorIndex[] = {
        0, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11,
        12, 13, 14, 15, 16,17,
        18, 19, 20, 21, 22, 23,
};

static const float CubePoints[]  = {
        // Front face
        -1.0f, 1.0f, 1.0f,            //pos
        0.0f, 0.5273f, 0.2656f, 1.0f,   //color
        0.0f, 0.0f, 1.0f,             //normal

        -1.0f, -1.0f, 1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, 1.0f,

        -1.0f, -1.0f, 1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, 1.0f,

        1.0f, -1.0f, 1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // Right face
        1.0f, 1.0f, 1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, -1.0f, 1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, -1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, -1.0f, 1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, -1.0f, -1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, -1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        1.0f, 0.0f, 0.0f,


        // Back face
        1.0f, 1.0f, -1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, -1.0f,

        1.0f, -1.0f, -1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, -1.0f,

        -1.0f, 1.0f, -1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, -1.0f,

        1.0f, -1.0f, -1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, -1.0f,

        -1.0f, 1.0f, -1.0f,
        0.0f, 0.5273f, 0.2656f, 1.0f,
        0.0f, 0.0f, -1.0f,

        // Left face
        -1.0f, 1.0f, -1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        -1.0f, 0.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        -1.0f, 0.0f, 0.0f,

        -1.0f, 1.0f, 1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        -1.0f, 0.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        -1.0f, 0.0f, 0.0f,

        -1.0f, -1.0f, 1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        -1.0f, 0.0f, 0.0f,

        -1.0f, 1.0f, 1.0f,
        0.0f, 0.3398f, 0.9023f, 1.0f,
        -1.0f, 0.0f, 0.0f,

        // Top face
        -1.0f, 1.0f, -1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, 1.0f, 0.0f,

        1.0f, 1.0f, -1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, 1.0f, 0.0f,

        1.0f, 1.0f, 1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, 1.0f, 0.0f,

        1.0f, 1.0f, -1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, 1.0f, 0.0f,

        // Bottom face
        1.0f, -1.0f, -1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, -1.0f, 0.0f,

        1.0f, -1.0f, 1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, -1.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, -1.0f, 0.0f,

        1.0f, -1.0f, 1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, -1.0f, 0.0f,

        -1.0f, -1.0f, 1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, -1.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,
        0.8359375f,  0.17578125f,  0.125f, 1.0f,
        0.0f, -1.0f, 0.0f,
};

static const unsigned short CubeIndex[] = {
        0, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11,
        12, 13, 14, 15, 16,17,
        18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35,
};

GlGeometry GlGeometry_CreateFloor() {
    GlGeometry geometry;

    geometry.vertexCount = sizeof(FloorIndex) / sizeof(FloorIndex[0]);
    geometry.indexCount = sizeof(FloorIndex) / sizeof(FloorIndex[0]);

    glGenBuffers(1, &geometry.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(FloorPoints), FloorPoints, GL_STATIC_DRAW);
    //LOG("DEBUG0407: sizeof(FloorPoints) = %d", sizeof(FloorPoints));

    glGenBuffers(1, &geometry.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FloorIndex), FloorIndex, GL_STATIC_DRAW);
    //LOG("DEBUG0407: sizeof(FloorIndex) = %d", sizeof(FloorIndex));


    glGenVertexArrays(1, &geometry.vertexArrayObject);
    LOG("DEBUG0407: geometry.vertexArrayObject = %d", geometry.vertexArrayObject);
    glBindVertexArray(geometry.vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, geometry.vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.indexBuffer);

    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_POSITION);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_LOCATION_POSITION, FLOOR_VERTEX_POS_SIZE, GL_FLOAT, false,
                          sizeof(float)*(FLOOR_VERTEX_POS_SIZE+FLOOR_VERTEX_COLOR_SIZE+FLOOR_VERTEX_NORMAL_SIZE),
                          (const GLvoid *) (sizeof(float) * 0));

    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_COLOR);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_LOCATION_COLOR, FLOOR_VERTEX_COLOR_SIZE, GL_FLOAT, false,
                          sizeof(float)*(FLOOR_VERTEX_POS_SIZE+FLOOR_VERTEX_COLOR_SIZE+FLOOR_VERTEX_NORMAL_SIZE),
                          (const GLvoid *) (sizeof(float) * FLOOR_VERTEX_POS_SIZE));
    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_NORMAL);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_LOCATION_NORMAL, FLOOR_VERTEX_NORMAL_SIZE, GL_FLOAT, false,
                          sizeof(float)*(FLOOR_VERTEX_POS_SIZE+FLOOR_VERTEX_COLOR_SIZE+FLOOR_VERTEX_NORMAL_SIZE),
                          (const GLvoid *) (sizeof(float) * (FLOOR_VERTEX_POS_SIZE+FLOOR_VERTEX_COLOR_SIZE)));
    glBindVertexArray(0);
    return geometry;
}

GlGeometry GlGeometry_CreateCube() {
    GlGeometry geometry;
    geometry.vertexCount = sizeof(CubeIndex) / sizeof(CubeIndex[0]);
    geometry.indexCount = sizeof(CubeIndex) / sizeof(CubeIndex[0]);

    glGenBuffers(1, &geometry.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubePoints), CubePoints, GL_STATIC_DRAW);

    glGenBuffers(1, &geometry.indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndex), CubeIndex, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_POSITION);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_LOCATION_POSITION, CUBE_VERTEX_POS_SIZE, GL_FLOAT, false,
                          sizeof(float)*(CUBE_VERTEX_POS_SIZE+CUBE_VERTEX_COLOR_SIZE+CUBE_VERTEX_NORMAL_SIZE),
                          (const GLvoid *) (sizeof(float) * 0));

    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_COLOR);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_LOCATION_COLOR, CUBE_VERTEX_COLOR_SIZE, GL_FLOAT, false,
                          sizeof(float)*(CUBE_VERTEX_POS_SIZE+CUBE_VERTEX_COLOR_SIZE+CUBE_VERTEX_NORMAL_SIZE),
                          (const GLvoid *) (sizeof(float) * CUBE_VERTEX_POS_SIZE));

    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_NORMAL);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_LOCATION_NORMAL, CUBE_VERTEX_NORMAL_SIZE, GL_FLOAT, false,
                          sizeof(float)*(CUBE_VERTEX_POS_SIZE+CUBE_VERTEX_COLOR_SIZE+CUBE_VERTEX_NORMAL_SIZE),
                          (const GLvoid *) (sizeof(float) * (CUBE_VERTEX_POS_SIZE+CUBE_VERTEX_COLOR_SIZE)));

    glBindVertexArray(0);
    return geometry;
}

void DrawModel(GlGeometry& glMesh, GlProgram& glProg, float x, float y, float z, float w,float px, float py, float pz,
               float mx, float my, float mz, int window_width, int window_height){

    Vector4f lightInWord(0.0f, 2.0f, 0.0f, 1.0f);
    Matrix4f M(
            1.0f, 0.0f, 0.0f, mx,
            0.0f, 1.0f, 0.0f, my,
            0.0f, 0.0f, 1.0f, mz,
            0.0f, 0.0f, 0.0f, 1.0f );
    Matrix4f V = Matrix4f(
            1.0f, 0.0f, 0.0f, px,
            0.0f, 1.0f, 0.0f, py,
            0.0f, 0.0f, 1.0f, pz,
            0.0f, 0.0f, 0.0f, 1.0f ) *
            Matrix4f( Quatf(x,y,z,w) );
    glViewport(0,0,window_width,window_height);
    Matrix4f P = Matrix4f::PerspectiveRH(M_PI/2, (float)window_width/(float)window_height, 0.1f, 10000.f);
    Vector4f lightInView = V.Transform(lightInWord);
    glUseProgram( glProg.program );
    if ( glProg.uM >= 0 )
    {
        glUniformMatrix4fv( glProg.uM, 1, GL_FALSE, M.Transposed().M[0] );
    }
    if ( glProg.uMv >= 0 )
    {
        glUniformMatrix4fv( glProg.uMv, 1, GL_FALSE, (V*M).Transposed().M[0] );
    }
    if ( glProg.uMvp >= 0 )
    {
        glUniformMatrix4fv( glProg.uMvp, 1, GL_FALSE, (P*V*M).Transposed().M[0] );
    }
    float LightPos[3]={lightInView.x,lightInView.y,lightInView.z};
    if ( glProg.uLightPos > 0 )
    {
        glUniform3fv( glProg.uLightPos, 1,  LightPos);
    }

    const int indexCount = glMesh.indexCount;
    LOG("WZJGL DrawModel indexCount = %d", indexCount);
    LOG("DEBUG0407: glMesh.vertexArrayObject = %d", glMesh.vertexArrayObject);
    glBindVertexArray(glMesh.vertexArrayObject);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}