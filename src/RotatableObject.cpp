#include "RotatableObject.hpp"

#include <glm/glm.hpp>

RotatableObject::RotatableObject() {
    // ctor
    rotation[0] = 0.0F;
    rotation[1] = 0.0F;
    rotation[2] = 0.0F;
}

RotatableObject::~RotatableObject() {
    // dtor
}

void RotatableObject::SetRotation(float x, float y, float z) {
    rotation[0] = x;
    rotation[1] = y;
    rotation[2] = z;
}

void RotatableObject::SetRotation(glm::vec3 rot) {
    rotation[0] = rot.x;
    rotation[1] = rot.y;
    rotation[2] = rot.z;
}
glm::vec3 RotatableObject::GetRotation() {
    return {rotation[0], rotation[1], rotation[2]};
}
