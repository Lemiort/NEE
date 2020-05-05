#ifndef INCLUDE_SCALEABLEOBJECT_H
#define INCLUDE_SCALEABLEOBJECT_H
#include <glm/glm.hpp>

class ScaleableObject {
public:
    ScaleableObject();
    virtual ~ScaleableObject();
    void SetScale(float x, float y, float z);
    void SetScale(glm::vec3 v);
    glm::vec3 GetScale();

protected:
    float scale[3];

private:
};

#endif  // INCLUDE_SCALEABLEOBJECT_H
