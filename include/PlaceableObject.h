#ifndef PLACEABLEOBJECT_H
#define PLACEABLEOBJECT_H

#include <Object.h>
#include <math_3d.h>

class PlaceableObject : public Object {
public:
    PlaceableObject();
    virtual ~PlaceableObject();
    float position[3];

    void SetPosition(float x, float y, float z);
    void SetPosition(Vector3f _pos);
    Vector3f GetPosition();

protected:
private:
};

#endif  // PLACEABLEOBJECT_H
