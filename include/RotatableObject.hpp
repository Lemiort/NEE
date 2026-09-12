#ifndef ROTATABLEOBJECT_H
#define ROTATABLEOBJECT_H
#include "Math3d.hpp"

class RotatableObject {
public:
    RotatableObject();
    virtual ~RotatableObject();
    void SetRotation(Vector3f rot);
    void SetRotation(float x, float y, float z);
    Vector3f GetRotation();

protected:
    float rotation[3];

private:
};

#endif  // ROTATABLEOBJECT_H
