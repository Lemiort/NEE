#include "PlaceableObject.hpp"

#include "Math3d.hpp"

PlaceableObject::PlaceableObject() {
    // ctor
    position[0] = 0.0F;
    position[1] = 0.0F;
    position[2] = 0.0F;
}

PlaceableObject::~PlaceableObject() {
    // dtor
}

void PlaceableObject::SetPosition(float x, float y, float z) {
    position[0] = x;
    position[1] = y;
    position[2] = z;
}
void PlaceableObject::SetPosition(Vector3f _pos) {
    position[0] = _pos.x;
    position[1] = _pos.y;
    position[2] = _pos.z;
}

Vector3f PlaceableObject::GetPosition() {
    return {position[0], position[1], position[2]};
}
