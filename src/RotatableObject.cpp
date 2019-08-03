#include "RotatableObject.h"

RotatableObject::RotatableObject() {
    // ctor
    rotation[0] = 0.0f;
    rotation[1] = 0.0f;
    rotation[2] = 0.0f;
}

RotatableObject::~RotatableObject() {
    // dtor
}

void RotatableObject::SetRotation(float x, float y, float z) {
    rotation[0] = x;
    rotation[1] = y;
    rotation[2] = z;
}

void RotatableObject::SetRotation(Vector3f rot) {
    rotation[0] = rot.x;
    rotation[1] = rot.y;
    rotation[2] = rot.z;
}
Vector3f RotatableObject::GetRotation() {
    return Vector3f(rotation[0], rotation[1], rotation[2]);
}
