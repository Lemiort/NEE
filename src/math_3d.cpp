#define _USE_MATH_DEFINES  // for C++
#include "math_3d.h"
#include <cmath>
#include <cstdlib>

Quaternion::Quaternion(float _x, float _y, float _z, float _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

void Quaternion::Normalize() {
    float Length = sqrtf(x * x + y * y + z * z + w * w);

    x /= Length;
    y /= Length;
    z /= Length;
    w /= Length;
}

Quaternion Quaternion::Conjugate() {
    Quaternion ret(-x, -y, -z, w);
    return ret;
}

Quaternion operator*(const Quaternion& l, const Quaternion& r) {
    const float w = (l.w * r.w) - (l.x * r.x) - (l.y * r.y) - (l.z * r.z);
    const float x = (l.x * r.w) + (l.w * r.x) + (l.y * r.z) - (l.z * r.y);
    const float y = (l.y * r.w) + (l.w * r.y) + (l.z * r.x) - (l.x * r.z);
    const float z = (l.z * r.w) + (l.w * r.z) + (l.x * r.y) - (l.y * r.x);

    Quaternion ret(x, y, z, w);

    return ret;
}

Quaternion operator*(const Quaternion& q, const Vector3f& v) {
    const float w = -(q.x * v.x) - (q.y * v.y) - (q.z * v.z);
    const float x = (q.w * v.x) + (q.y * v.z) - (q.z * v.y);
    const float y = (q.w * v.y) + (q.z * v.x) - (q.x * v.z);
    const float z = (q.w * v.z) + (q.x * v.y) - (q.y * v.x);

    Quaternion ret(x, y, z, w);

    return ret;
}


namespace glm
{
    Vector3f cross(const Vector3f& a, const Vector3f& b){
        auto result = glm::cross(a.to_vec3(), b.to_vec3());
        return Vector3f(result);
    }
    Vector3f rotate(const Vector3f& v,const float angle, const Vector3f& normal){
        return Vector3f(glm::rotate(v.to_vec3(), angle, normal.to_vec3()));
    }

    Vector3f normalize(const Vector3f& v){
        return glm::normalize(v.to_vec3());
    }

    float length(const Vector3f& v){
        return glm::length(v.to_vec3());
    }

    float angle(Vector3f left, Vector3f right){
        return glm::angle(left.to_vec3(), right.to_vec3());
    }

    Matrix4f scale(const Vector3f& v){
        auto mat = glm::scale(glm::vec3(v.x, v.y, v.z));
        return mat;
    }

    Matrix4f translate(const Vector3f& v){
        auto mat = glm::translate(v.to_vec3());
        mat = glm::transpose(mat);
        return mat;
    }

    Matrix4f perspective(float FOV, float Width, float Height, float zNear, float zFar){
        auto mat = glm::perspectiveFov(FOV,Width,Height,zNear, zFar);
        mat = glm::transpose(mat);
        return mat;
    }

    Matrix4f lookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up){
        auto mat = glm::lookAt(eye.to_vec3(), center.to_vec3(), up.to_vec3());
        mat = glm::transpose(mat);
        return mat;
    }

    Matrix4f orientate4(const Vector3f& v){
        auto mat = glm::orientate4(v.to_vec3());
        mat = glm::transpose(mat);
        return mat;
    }

} // namespace glm
