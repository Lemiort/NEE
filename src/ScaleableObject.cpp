#include "ScaleableObject.h"

ScaleableObject::ScaleableObject() {
    // ctor
}

ScaleableObject::~ScaleableObject() {
    // dtor
}

void ScaleableObject::SetScale(glm::vec3 v) {
    scale[0] = v.x;
    scale[1] = v.y;
    scale[2] = v.z;
}

void ScaleableObject::SetScale(float x, float y, float z) {
    scale[0] = x;
    scale[1] = y;
    scale[2] = z;
}

glm::vec3 ScaleableObject::GetScale() {
    return glm::vec3(scale[0], scale[1], scale[2]);
}
