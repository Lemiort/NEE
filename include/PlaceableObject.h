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
    void SetPosition(glm::vec3 _pos);
    glm::vec3 GetPosition();

protected:
private:
};

#endif  // PLACEABLEOBJECT_H
