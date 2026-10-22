#ifndef PLACEABLEOBJECT_H
#define PLACEABLEOBJECT_H

#include <array>
#include <glm/ext/vector_float3.hpp>

#include "Object.hpp"

class PlaceableObject : public Object {
public:
    PlaceableObject();
    ~PlaceableObject() override;
    std::array<float, 3> position{0.0F, 0.0F, 0.0F};

    void SetPosition(float x, float y, float z);
    void SetPosition(glm::vec3 _pos);
    glm::vec3 GetPosition();

protected:
private:
};

#endif  // PLACEABLEOBJECT_H
