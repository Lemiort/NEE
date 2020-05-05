#ifndef ROTATABLEOBJECT_H
#define ROTATABLEOBJECT_H
#include <glm/glm.hpp>

class RotatableObject {
public:
    RotatableObject();
    virtual ~RotatableObject();
    void SetRotation(glm::vec3 rot);
    void SetRotation(float x, float y, float z);
    glm::vec3 GetRotation();

protected:
    float rotation[3];

private:
};

#endif  // ROTATABLEOBJECT_H
