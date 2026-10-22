#include "RotatableObject.hpp"

#include <glm/ext/vector_float3.hpp>

RotatableObject::RotatableObject() = default;

RotatableObject::~RotatableObject() = default;

void RotatableObject::SetRotation(float x, float y, float z) {
    rotation.at(0) = x;
    rotation.at(1) = y;
    rotation.at(2) = z;
}

void RotatableObject::SetRotation(glm::vec3 rot) {
    rotation.at(0) = rot.x;
    rotation.at(1) = rot.y;
    rotation.at(2) = rot.z;
}
glm::vec3 RotatableObject::GetRotation() {
    return {rotation.at(0), rotation.at(1), rotation.at(2)};
}
