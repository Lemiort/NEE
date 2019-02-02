#include "PlaceableObject.h"

PlaceableObject::PlaceableObject()
{
    //ctor
    position[0] = 0.0f;
    position[1] = 0.0f;
    position[2] = 0.0f;
}

PlaceableObject::~PlaceableObject()
{
    //dtor
}


void PlaceableObject::SetPosition(float x, float y, float z)
{
    position[0] = x;
    position[1] = y;
    position[2] = z;
}
void PlaceableObject::SetPosition(Vector3f _pos)
{
    position[0] = _pos.x;
    position[1] = _pos.y;
    position[2] = _pos.z;
}

Vector3f PlaceableObject::GetPosition()
{
    return Vector3f(position[0],
                    position[1],
                    position[2]);
}
