#ifndef GlProgram_h
#define GlProgram_h

#include "GlUtils.h"
#include <math.h>

template<class T>
class Vector4 {
public:
    T x, y, z, w;

    // FIXME: default initialization of a vector class can be very expensive in a full-blown
    // application.  A few hundred thousand vector constructions is not unlikely and can add
    // up to milliseconds of time on certain processors.
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) { }
};

typedef Vector4<float>  Vector4f;
typedef Vector4<double> Vector4d;
typedef Vector4<int>    Vector4i;


template<class T>
class Quat {
public:
    // w + Xi + Yj + Zk
    T x, y, z, w;

    Quat() : x(0), y(0), z(0), w(1) {}
    Quat(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) { }
};
typedef Quat<float>  Quatf;
typedef Quat<double> Quatd;
template<class T>
class Matrix4
{
    static const Matrix4 IdentityValue;

public:
    T M[4][4];

    enum NoInitType { NoInit };

    // Construct with no memory initialization.
    Matrix4(NoInitType) { }

    // By default, we construct identity matrix.
    Matrix4()
    {
        SetIdentity();
    }

    Matrix4(T m11, T m12, T m13, T m14,
            T m21, T m22, T m23, T m24,
            T m31, T m32, T m33, T m34,
            T m41, T m42, T m43, T m44)
    {
        M[0][0] = m11; M[0][1] = m12; M[0][2] = m13; M[0][3] = m14;
        M[1][0] = m21; M[1][1] = m22; M[1][2] = m23; M[1][3] = m24;
        M[2][0] = m31; M[2][1] = m32; M[2][2] = m33; M[2][3] = m34;
        M[3][0] = m41; M[3][1] = m42; M[3][2] = m43; M[3][3] = m44;
    }

    Matrix4(T m11, T m12, T m13,
            T m21, T m22, T m23,
            T m31, T m32, T m33)
    {
        M[0][0] = m11; M[0][1] = m12; M[0][2] = m13; M[0][3] = 0;
        M[1][0] = m21; M[1][1] = m22; M[1][2] = m23; M[1][3] = 0;
        M[2][0] = m31; M[2][1] = m32; M[2][2] = m33; M[2][3] = 0;
        M[3][0] = 0;   M[3][1] = 0;   M[3][2] = 0;   M[3][3] = 1;
    }

    explicit Matrix4(const Quat<T>& q)
    {
        FromQuat( q );
    }

    void FromQuat( const Quat<T> & q )
    {
        T ww = q.w*q.w;
        T xx = q.x*q.x;
        T yy = q.y*q.y;
        T zz = q.z*q.z;

        M[0][0] = ww + xx - yy - zz;       M[0][1] = 2 * (q.x*q.y - q.w*q.z); M[0][2] = 2 * (q.x*q.z + q.w*q.y); M[0][3] = 0;
        M[1][0] = 2 * (q.x*q.y + q.w*q.z); M[1][1] = ww - xx + yy - zz;       M[1][2] = 2 * (q.y*q.z - q.w*q.x); M[1][3] = 0;
        M[2][0] = 2 * (q.x*q.z - q.w*q.y); M[2][1] = 2 * (q.y*q.z + q.w*q.x); M[2][2] = ww - xx - yy + zz;       M[2][3] = 0;
        M[3][0] = 0;                       M[3][1] = 0;                       M[3][2] = 0;                       M[3][3] = 1;
    }

    static const Matrix4& Identity()  { return IdentityValue; }

    void SetIdentity()
    {
        M[0][0] = M[1][1] = M[2][2] = M[3][3] = 1;
        M[0][1] = M[1][0] = M[2][3] = M[3][1] = 0;
        M[0][2] = M[1][2] = M[2][0] = M[3][2] = 0;
        M[0][3] = M[1][3] = M[2][1] = M[3][0] = 0;
    }

    bool operator== (const Matrix4& b) const
    {
        bool isEqual = true;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                isEqual &= (M[i][j] == b.M[i][j]);

        return isEqual;
    }

    Matrix4 operator+ (const Matrix4& b) const
    {
        Matrix4 result(*this);
        result += b;
        return result;
    }

    Matrix4& operator+= (const Matrix4& b)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] += b.M[i][j];
        return *this;
    }

    Matrix4 operator- (const Matrix4& b) const
    {
        Matrix4 result(*this);
        result -= b;
        return result;
    }

    Matrix4& operator-= (const Matrix4& b)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] -= b.M[i][j];
        return *this;
    }

    // Multiplies two matrices into destination with minimum copying.
    // FIXME: take advantage of return value optimization instead.
    static Matrix4& Multiply(Matrix4* d, const Matrix4& a, const Matrix4& b)
    {
        int i = 0;
        do {
            d->M[i][0] = a.M[i][0] * b.M[0][0] + a.M[i][1] * b.M[1][0] + a.M[i][2] * b.M[2][0] + a.M[i][3] * b.M[3][0];
            d->M[i][1] = a.M[i][0] * b.M[0][1] + a.M[i][1] * b.M[1][1] + a.M[i][2] * b.M[2][1] + a.M[i][3] * b.M[3][1];
            d->M[i][2] = a.M[i][0] * b.M[0][2] + a.M[i][1] * b.M[1][2] + a.M[i][2] * b.M[2][2] + a.M[i][3] * b.M[3][2];
            d->M[i][3] = a.M[i][0] * b.M[0][3] + a.M[i][1] * b.M[1][3] + a.M[i][2] * b.M[2][3] + a.M[i][3] * b.M[3][3];
        } while((++i) < 4);

        return *d;
    }

    Matrix4 operator* (const Matrix4& b) const
    {
        Matrix4 result(Matrix4::NoInit);
        Multiply(&result, *this, b);
        return result;
    }

    Matrix4& operator*= (const Matrix4& b)
    {
        return Multiply(this, Matrix4(*this), b);
    }

    Matrix4 operator* (T s) const
    {
        Matrix4 result(*this);
        result *= s;
        return result;
    }

    Matrix4& operator*= (T s)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] *= s;
        return *this;
    }


    Matrix4 operator/ (T s) const
    {
        Matrix4 result(*this);
        result /= s;
        return result;
    }

    Matrix4& operator/= (T s)
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                M[i][j] /= s;
        return *this;
    }


    Vector4<T> Transform(const Vector4<T>& v) const
    {
        return Vector4<T>(M[0][0] * v.x + M[0][1] * v.y + M[0][2] * v.z + M[0][3] * v.w,
                          M[1][0] * v.x + M[1][1] * v.y + M[1][2] * v.z + M[1][3] * v.w,
                          M[2][0] * v.x + M[2][1] * v.y + M[2][2] * v.z + M[2][3] * v.w,
                          M[3][0] * v.x + M[3][1] * v.y + M[3][2] * v.z + M[3][3] * v.w);
    }

    Matrix4 Transposed() const
    {
        return Matrix4(M[0][0], M[1][0], M[2][0], M[3][0],
                       M[0][1], M[1][1], M[2][1], M[3][1],
                       M[0][2], M[1][2], M[2][2], M[3][2],
                       M[0][3], M[1][3], M[2][3], M[3][3]);
    }

    void     Transpose()
    {
        *this = Transposed();
    }

    // Creates a matrix for scaling by vector
    static Matrix4 Scaling(T x, T y, T z)
    {
        Matrix4 t;
        t.M[0][0] = x;
        t.M[1][1] = y;
        t.M[2][2] = z;
        return t;
    }

    // Creates a matrix for scaling by constant
    static Matrix4 Scaling(T s)
    {
        Matrix4 t;
        t.M[0][0] = s;
        t.M[1][1] = s;
        t.M[2][2] = s;
        return t;
    }

    // PerspectiveRH creates a right-handed perspective projection matrix that can be
    // used with the sample renderer.
    //  yfov   - Specifies vertical field of view in radians.
    //  aspect - Screen aspect ration, which is usually width/height for square pixels.
    //           Note that xfov = yfov * aspect.
    //  znear  - Absolute value of near Z clipping clipping range.
    //  zfar   - Absolute value of far  Z clipping clipping range (larger then near).
    // Even though RHS usually looks in the direction of negative Z, positive values
    // are expected for znear and zfar.
    static Matrix4 PerspectiveRH(T yfov, T aspect, T znear, T zfar)
    {
        Matrix4 m;
        T tanHalfFov = tan(yfov * T(0.5f));

        m.M[0][0] = T(1) / (aspect * tanHalfFov);
        m.M[1][1] = T(1) / tanHalfFov;
        m.M[2][2] = zfar / (znear - zfar);
        // m.M[2][2] = zfar / (zfar - znear);
        m.M[3][2] = T(-1);
        m.M[2][3] = (zfar * znear) / (znear - zfar);
        m.M[3][3] = T(0);

        // Note: Post-projection matrix result assumes Left-Handed coordinate system,
        //       with Y up, X right and Z forward. This supports positive z-buffer values.
        // This is the case even for RHS cooridnate input.
        return m;
    }

    // PerspectiveLH creates a left-handed perspective projection matrix that can be
    // used with the sample renderer.
    //  yfov   - Specifies vertical field of view in radians.
    //  aspect - Screen aspect ration, which is usually width/height for square pixels.
    //           Note that xfov = yfov * aspect.
    //  znear  - Absolute value of near Z clipping clipping range.
    //  zfar   - Absolute value of far  Z clipping clipping range (larger then near).
    static Matrix4 PerspectiveLH(T yfov, T aspect, T znear, T zfar)
    {
        Matrix4 m;
        T tanHalfFov = tan(yfov * T(0.5f));

        m.M[0][0] = T(1) / (aspect * tanHalfFov);
        m.M[1][1] = T(1) / tanHalfFov;
        m.M[2][2] = zfar / (zfar - znear);
        m.M[3][2] = T(1);
        m.M[2][3] = (zfar * znear) / (znear - zfar);
        m.M[3][3] = T(0);

        // Note: Post-projection matrix result assumes Left-Handed coordinate system,
        //       with Y up, X right and Z forward. This supports positive z-buffer values.
        return m;
    }

    static Matrix4 Ortho2D(T w, T h)
    {
        Matrix4 m;
        m.M[0][0] = T(2) / w;
        m.M[1][1] = T(-2) / h;
        m.M[0][3] = T(-1);
        m.M[1][3] = T(1);
        m.M[2][2] = T(0);
        return m;
    }
};

typedef Matrix4<float>  Matrix4f;
typedef Matrix4<double> Matrix4d;

static const char* vertexShaderFloor =
        "#version 300 es                                                              \n"
        "uniform mat4 u_Model;                                                        \n"
        "uniform mat4 u_MVP;                                                          \n"
        "uniform mat4 u_MVMatrix;                                                     \n"
        "uniform vec3 u_LightPos;                                                     \n"
        "layout(location = 0) in vec4 a_Position;                                     \n"
        "layout(location = 1) in vec4 a_Color;                                        \n"
        "layout(location = 2) in vec3 a_Normal;                                       \n"
        "out vec4 v_Color;                                                            \n"
        "out vec3 v_Grid;                                                             \n"
        "void main() {                                                                \n"
        "    v_Grid = vec3(u_Model * a_Position);                                     \n"
        "    vec3 modelViewVertex = vec3(u_MVMatrix * a_Position);                    \n"
        "    vec3 modelViewNormal = vec3(u_MVMatrix * vec4(a_Normal, 0.0));           \n"
        "    float distance = length(u_LightPos - modelViewVertex);                   \n"
        "    vec3 lightVector = normalize(u_LightPos - modelViewVertex);              \n"
        "    float diffuse = max(dot(modelViewNormal, lightVector), 0.5);             \n"
        "    diffuse = diffuse * (1.0 / (1.0 + (0.00001 * distance * distance)));     \n"
        "    v_Color = vec4(a_Color.rgb * diffuse, a_Color.a);                        \n"
        "    gl_Position = u_MVP * a_Position;                                        \n"
        "}                                                                            \n";

static const char* fragmentShaderFloor =
        "#version 300 es                                                                      \n"
        "precision mediump float;                                                             \n"
        "in vec4 v_Color;                                                                     \n"
        "in vec3 v_Grid;                                                                      \n"
        "out vec4 o_fragColor;                                                                \n"
        "void main() {                                                                        \n"
        "    float depth = gl_FragCoord.z / gl_FragCoord.w; // Calculate world-space distance.\n"
        "    vec4 v_Color_m = (1000.0 - depth) / 1000.0 * v_Color;                            \n"
        "    if ((mod(abs(v_Grid.x), 10.0) < 0.1) || (mod(abs(v_Grid.z), 10.0) < 0.1)) {      \n"
        "       o_fragColor = max(0.0, (90.0-depth) / 90.0) * vec4(1.0, 1.0, 1.0, 1.0)        \n"
        "                     + min(1.0, depth / 90.0) * v_Color_m;                           \n"
        "    } else {                                                                         \n"
        "       o_fragColor = v_Color_m;                                                      \n"
        "    }                                                                                \n"
        "}                                                                                    \n";

static const char* vertexShaderCube =
        "#version 300 es                                                              \n"
        "uniform mat4 u_Model;                                                        \n"
        "uniform mat4 u_MVP;                                                          \n"
        "uniform mat4 u_MVMatrix;                                                     \n"
        "uniform vec3 u_LightPos;                                                     \n"
        "layout(location = 0) in vec4 a_Position;                                     \n"
        "layout(location = 1) in vec4 a_Color;                                        \n"
        "layout(location = 2) in vec3 a_Normal;                                       \n"
        "out vec4 v_Color;                                                            \n"
        "out vec3 v_Grid;                                                             \n"
        "void main() {                                                                \n"
        "    v_Grid = vec3(u_Model * a_Position);                                     \n"
        "    vec3 modelViewVertex = vec3(u_MVMatrix * a_Position);                    \n"
        "    vec3 modelViewNormal = vec3(u_MVMatrix * vec4(a_Normal, 0.0));           \n"
        "    float distance = length(u_LightPos - modelViewVertex);                   \n"
        "    vec3 lightVector = normalize(u_LightPos - modelViewVertex);              \n"
        "    float diffuse = max(dot(modelViewNormal, lightVector), 0.5);             \n"
        "    diffuse = diffuse * (1.0 / (1.0 + (0.00001 * distance * distance)));     \n"
        "    v_Color = vec4(a_Color.rgb * diffuse, a_Color.a);                        \n"
        "    gl_Position = u_MVP * a_Position;                                        \n"
        "}                                                                            \n";
static const char* fragmentShaderCube =
        "#version 300 es                                                              \n"
        "precision mediump float;                                                     \n"
        "in vec4 v_Color;                                                             \n"
        "out vec4 o_fragColor;                                                        \n"
        "void main()                                                                  \n"
        "{                                                                            \n"
        "   o_fragColor = v_Color;                                                     \n"
        "}                                                                            \n";
enum VertexAttributeLocation {
    VERTEX_ATTRIBUTE_LOCATION_POSITION = 0,
    VERTEX_ATTRIBUTE_LOCATION_COLOR = 1,
    VERTEX_ATTRIBUTE_LOCATION_NORMAL = 2,
};

struct GlProgram {
    GlProgram() :
            program(0),
            vertexShader(0),
            fragmentShader(0),
            uM(-1),
            uMv(-1),
            uMvp(-1),
            uLightPos(-1) {}

    // These will always be > 0 after a build, any errors will abort()
    GLuint program;
    GLuint vertexShader;
    GLuint fragmentShader;

    // Uniforms that aren't found will have a -1 value

    GLint uM;                // uniform Modelm
    GLint uMv;                // uniform Viewm
    GLint uLightPos;        // uniform Projectionm
    GLint uMvp;                // uniform Mvpm
};

// Will abort() after logging an error if either compiles or the link status
// fails, but not if uniforms are missing.
void GlProgram_Create(GlProgram *prog, const char *vertexSrc, const char *fragmentSrc);

void GlProgram_Destroy(GlProgram *prog);


#endif    // GlProgram_h
