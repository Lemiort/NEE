#ifndef PLACEABLEOBJECT_H
#define PLACEABLEOBJECT_H

#include "Math3d.hpp"
#include "Object.hpp"

class PlaceableObject : public Object {
public:
    PlaceableObject();
    ~PlaceableObject() override;
    float position[3]{};

    void SetPosition(float x, float y, float z);
    void SetPosition(Vector3f _pos);
    Vector3f GetPosition();

protected:
private:
};

#endif  // PLACEABLEOBJECT_H
