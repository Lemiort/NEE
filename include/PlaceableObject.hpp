#ifndef PLACEABLEOBJECT_H
#define PLACEABLEOBJECT_H

#include <array>

#include "Math3d.hpp"
#include "Object.hpp"

class PlaceableObject : public Object {
public:
    PlaceableObject();
    ~PlaceableObject() override;
    std::array<float, 3> position{0.0F, 0.0F, 0.0F};

    void SetPosition(float x, float y, float z);
    void SetPosition(Vector3f _pos);
    Vector3f GetPosition();

protected:
private:
};

#endif  // PLACEABLEOBJECT_H
