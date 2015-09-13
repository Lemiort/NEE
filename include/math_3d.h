#ifndef MATH_3D_H_INCLUDED
#define MATH_3D_H_INCLUDED
#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <cstdio>
#include <iostream>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif //M_PI

#define ToRadian(x) ((x) * M_PI / 180.0f)
#define ToDegree(x) ((x) * 180.0f / M_PI)

float RandomFloat();

class Vector2i
{
    public:
    int x;
    int y;
    Vector2i();
};

struct Vector2f
{
    float x;
    float y;

    Vector2f()
    {
        x =0;
        y =0;
    }

    Vector2f(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
};

struct Vector4f
{
    float r;
    float g;
    float b;
	float a;

    Vector4f()
    {
        r = 0;
        g = 0;
        b = 0;
        a = 0;
    }

    Vector4f(float _r, float _g, float _b, float _a)
    {
        r=_r;
		g=_g;
		b=_b;
		a=_a;
    }
};

struct Vector3f
{
    float x;
    float y;
    float z;

    Vector3f()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    Vector3f(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    Vector3f& operator+=(const Vector3f& r)
    {
        x += r.x;
        y += r.y;
        z += r.z;

        return *this;
    }

    Vector3f& operator-=(const Vector3f& r)
    {
        x -= r.x;
        y -= r.y;
        z -= r.z;

        return *this;
    }

    Vector3f& operator*=(float f)
    {
        x *= f;
        y *= f;
        z *= f;

        return *this;
    }

    Vector3f Cross(const Vector3f& v) const;

    Vector3f& Normalize();

    void Rotate(float Angle, const Vector3f& Axis);

    void Print() const
    {
        printf("(%.02f, %.02f, %.02f", x, y, z);
    }

    float Lenght();

};


inline Vector3f operator+(const Vector3f& l, const Vector3f& r)
{
    Vector3f Ret(l.x + r.x,
                 l.y + r.y,
                 l.z + r.z);

    return Ret;
}

inline Vector3f operator-(const Vector3f& l, const Vector3f& r)
{
    Vector3f Ret(l.x - r.x,
                 l.y - r.y,
                 l.z - r.z);

    return Ret;
}

inline Vector3f operator*(const Vector3f& l, float f)
{
    Vector3f Ret(l.x * f,
                 l.y * f,
                 l.z * f);

    return Ret;
}

inline float operator*(const Vector3f& l,const Vector3f& r)
{
    return l.x*r.x+l.y*r.y+l.z*r.z;
}

inline bool operator!=(const Vector3f& l,const Vector3f& r)
{
    if(fabs(l.x-r.x) < 1e-6 &&
       fabs(l.y-r.y) < 1e-6 &&
       fabs(l.z-r.z) < 1e-6)
        return false;
    else
        return true;
}

inline bool operator==(const Vector3f& l,const Vector3f& r)
{
    if(fabs(l.x-r.x) < 1e-6 &&
       fabs(l.y-r.y) < 1e-6 &&
       fabs(l.z-r.z) < 1e-6)
        return true;
    else
        return false;
}

class Matrix4f
{
public:
    float m[4][4];

    Matrix4f()
    {
    }


    inline void InitIdentity()
    {
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
        m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
    }

    inline Matrix4f operator*(const Matrix4f& Right) const
    {
        Matrix4f Ret;

        for (unsigned int i = 0 ; i < 4 ; i++) {
            for (unsigned int j = 0 ; j < 4 ; j++) {
                Ret.m[i][j] = m[i][0] * Right.m[0][j] +
                              m[i][1] * Right.m[1][j] +
                              m[i][2] * Right.m[2][j] +
                              m[i][3] * Right.m[3][j];
            }
        }

        return Ret;
    }

    void InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ);
    void InitRotateTransform(float RotateX, float RotateY, float RotateZ);
    void InitTranslationTransform(float x, float y, float z);
    void InitCameraTransform(const Vector3f& Target, const Vector3f& Up);
    void InitPersProjTransform(float FOV, float Width, float Height, float zNear, float zFar);
};
//return an angle in radians
inline float CalcAngle(Vector3f left, Vector3f right)
{
    float resultLength = left.Lenght() * right.Lenght();
    if( fabs(resultLength) < 1e-6)
        return 0;
    return acos(left * right / ( resultLength ));
}

//return coordinates in spherical type: r, x, z
inline Vector3f ToSphericalCoordinates(Vector3f v)
{
    return Vector3f(v.Lenght(),
                    atan( sqrt(v.x*v.x+v.y+v.y)/v.z),
                    atan(v.y/v.x));
}

struct Quaternion
{
    float x, y, z, w;

    Quaternion(float _x, float _y, float _z, float _w);

    void Normalize();

    Quaternion Conjugate();
 };

Quaternion operator*(const Quaternion& l, const Quaternion& r);

Quaternion operator*(const Quaternion& q, const Vector3f& v);
#endif //MTH3D_H_INCLUDED
