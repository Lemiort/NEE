#ifndef INCLUDE_ROTATABLEOBJECT_H
#define INCLUDE_ROTATABLEOBJECT_H
#include <glm/glm.hpp>

class RotatableObject {
public:
    RotatableObject();
    virtual ~RotatableObject();
    void SetRotation(glm::vec3 rot);
    void SetRotation(float x, float y, float z);
    glm::vec3 GetRotation();

protected:
    glm::vec3 model_rotation;

private:
};

#endif  // INCLUDE_ROTATABLEOBJECT_H
