#ifndef MATH_3D_H_INCLUDED
#define MATH_3D_H_INCLUDED
#define _USE_MATH_DEFINES  // for C++
#include <cmath>
#include <cstdio>
#include <iostream>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp> 
#include <glm/gtx/vector_angle.hpp> 
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/random.hpp>

struct Vector3f {
    float x;
    float y;
    float z;

    Vector3f(const glm::vec3& v) {
        x = v.x;
        y = v.y;
        z = v.z;
    }

    glm::vec3 to_vec3() const{
        return glm::vec3(x,y,z);
    }


    Vector3f() {
        x = 0;
        y = 0;
        z = 0;
    }

    Vector3f(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }

    Vector3f& operator+=(const Vector3f& r) {
        x += r.x;
        y += r.y;
        z += r.z;

        return *this;
    }

    Vector3f& operator-=(const Vector3f& r) {
        x -= r.x;
        y -= r.y;
        z -= r.z;

        return *this;
    }

    Vector3f& operator*=(float f) {
        x *= f;
        y *= f;
        z *= f;

        return *this;
    }

    void Print() const { printf("(%.02f, %.02f, %.02f", x, y, z); }
};

inline Vector3f operator+(const Vector3f& l, const Vector3f& r) {
    Vector3f Ret(l.x + r.x, l.y + r.y, l.z + r.z);

    return Ret;
}

inline Vector3f operator-(const Vector3f& l, const Vector3f& r) {
    Vector3f Ret(l.x - r.x, l.y - r.y, l.z - r.z);

    return Ret;
}

inline Vector3f operator-( const Vector3f& r) {
    Vector3f Ret(-r.x,-r.y,-r.z);
    return Ret;
}

inline Vector3f operator*(const Vector3f& l, float f) {
    Vector3f Ret(l.x * f, l.y * f, l.z * f);

    return Ret;
}

inline float operator*(const Vector3f& l, const Vector3f& r) {
    return l.x * r.x + l.y * r.y + l.z * r.z;
}

inline bool operator!=(const Vector3f& l, const Vector3f& r) {
    if (fabs(l.x - r.x) < 1e-6 && fabs(l.y - r.y) < 1e-6 &&
        fabs(l.z - r.z) < 1e-6)
        return false;
    else
        return true;
}

inline bool operator==(const Vector3f& l, const Vector3f& r) {
    if (fabs(l.x - r.x) < 1e-6 && fabs(l.y - r.y) < 1e-6 &&
        fabs(l.z - r.z) < 1e-6)
        return true;
    else
        return false;
}

class Matrix4f {
public:
    float m[4][4];

    Matrix4f(glm::mat4 mat){
        m[0][0] = mat[0][0];
        m[0][1] = mat[0][1];
        m[0][2] = mat[0][2];
        m[0][3] = mat[0][3];
        m[1][0] = mat[1][0];
        m[1][1] = mat[1][1];
        m[1][2] = mat[1][2];
        m[1][3] = mat[1][3];
        m[2][0] = mat[2][0];
        m[2][1] = mat[2][1];
        m[2][2] = mat[2][2];
        m[2][3] = mat[2][3];
        m[3][0] = mat[3][0];
        m[3][1] = mat[3][1];
        m[3][2] = mat[3][2];
        m[3][3] = mat[3][3];
    }

    Matrix4f() {}

    glm::mat4 to_mat4(){
        glm::mat4 mat;
        mat[0][0] = m[0][0];
        mat[0][1] = m[0][1];
        mat[0][2] = m[0][2];
        mat[0][3] = m[0][3];
        mat[1][0] = m[1][0];
        mat[1][1] = m[1][1];
        mat[1][2] = m[1][2];
        mat[1][3] = m[1][3];
        mat[2][0] = m[2][0];
        mat[2][1] = m[2][1];
        mat[2][2] = m[2][2];
        mat[2][3] = m[2][3];
        mat[3][0] = m[3][0];
        mat[3][1] = m[3][1];
        mat[3][2] = m[3][2];
        mat[3][3] = m[3][3];
        return mat;
    }

    inline void InitIdentity() {
        m[0][0] = 1.0f;
        m[0][1] = 0.0f;
        m[0][2] = 0.0f;
        m[0][3] = 0.0f;
        m[1][0] = 0.0f;
        m[1][1] = 1.0f;
        m[1][2] = 0.0f;
        m[1][3] = 0.0f;
        m[2][0] = 0.0f;
        m[2][1] = 0.0f;
        m[2][2] = 1.0f;
        m[2][3] = 0.0f;
        m[3][0] = 0.0f;
        m[3][1] = 0.0f;
        m[3][2] = 0.0f;
        m[3][3] = 1.0f;
    }

    inline Matrix4f operator*(const Matrix4f& Right) const {
        Matrix4f Ret;

        for (unsigned int i = 0; i < 4; i++) {
            for (unsigned int j = 0; j < 4; j++) {
                Ret.m[i][j] = m[i][0] * Right.m[0][j] +
                              m[i][1] * Right.m[1][j] +
                              m[i][2] * Right.m[2][j] + m[i][3] * Right.m[3][j];
            }
        }

        return Ret;
    }
};

struct Quaternion {
    float x, y, z, w;

    Quaternion(float _x, float _y, float _z, float _w);

    void Normalize();

    Quaternion Conjugate();
};

Quaternion operator*(const Quaternion& l, const Quaternion& r);

Quaternion operator*(const Quaternion& q, const Vector3f& v);

namespace glm
{
    float length(const Vector3f& v);
    Vector3f cross(const Vector3f& a, const Vector3f& b);
    Vector3f rotate(const Vector3f& v,const float angle, const Vector3f& normal);
    Vector3f normalize(const Vector3f& v);
    float angle(Vector3f left, Vector3f right);
    Matrix4f scale(const Vector3f& v);
    Matrix4f translate(const Vector3f& v);
    Matrix4f perspective(float FOV, float Width, float Height, float zNear, float zFar);
    Matrix4f lookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up);
    Matrix4f orientate4(const Vector3f& v);
} // namespace glm


#endif  // MTH3D_H_INCLUDED
