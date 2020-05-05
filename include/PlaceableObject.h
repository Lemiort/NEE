#ifndef INCLUDE_PLACEABLEOBJECT_H_
#define INCLUDE_PLACEABLEOBJECT_H_

#include <glm/glm.hpp>

#include "Object.h"

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

#endif  // INCLUDE_PLACEABLEOBJECT_H_
