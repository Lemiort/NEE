#ifndef SCALEABLEOBJECT_H
#define SCALEABLEOBJECT_H
#include <array>

#include <glm/ext/vector_float3.hpp>

class ScaleableObject {
public:
    ScaleableObject();
    virtual ~ScaleableObject();
    void SetScale(float x, float y, float z);
    void SetScale(glm::vec3 v);
    glm::vec3 GetScale();

protected:
    std::array<float, 3> scale{};

private:
};

#endif  // SCALEABLEOBJECT_H
