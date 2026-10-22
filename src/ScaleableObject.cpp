#include "ScaleableObject.hpp"

#include <glm/ext/vector_float3.hpp>

ScaleableObject::ScaleableObject() = default;

ScaleableObject::~ScaleableObject() = default;

void ScaleableObject::SetScale(glm::vec3 v) {
    scale.at(0) = v.x;
    scale.at(1) = v.y;
    scale.at(2) = v.z;
}

void ScaleableObject::SetScale(float x, float y, float z) {
    scale.at(0) = x;
    scale.at(1) = y;
    scale.at(2) = z;
}

glm::vec3 ScaleableObject::GetScale() {
    return {scale.at(0), scale.at(1), scale.at(2)};
}
