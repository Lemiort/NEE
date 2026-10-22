#include "PlaceableObject.hpp"

#include <glm/ext/vector_float3.hpp>

PlaceableObject::PlaceableObject() = default;

PlaceableObject::~PlaceableObject() = default;

void PlaceableObject::SetPosition(float x, float y, float z) {
    position.at(0) = x;
    position.at(1) = y;
    position.at(2) = z;
}
void PlaceableObject::SetPosition(glm::vec3 _pos) {
    position.at(0) = _pos.x;
    position.at(1) = _pos.y;
    position.at(2) = _pos.z;
}

glm::vec3 PlaceableObject::GetPosition() {
    return {position.at(0), position.at(1), position.at(2)};
}
