#ifndef ROTATABLEOBJECT_H
#define ROTATABLEOBJECT_H
#include <array>

#include <glm/ext/vector_float3.hpp>

class RotatableObject {
public:
    RotatableObject();
    virtual ~RotatableObject();
    void SetRotation(glm::vec3 rot);
    void SetRotation(float x, float y, float z);
    glm::vec3 GetRotation();

protected:
    std::array<float, 3> rotation{};

private:
};

#endif  // ROTATABLEOBJECT_H
