#ifndef INCLUDE_PLACEABLEOBJECT_H
#define INCLUDE_PLACEABLEOBJECT_H

#include <glm/glm.hpp>

class PlaceableObject {
public:
    PlaceableObject() = default;
    virtual ~PlaceableObject() = default;
    void SetPosition(glm::vec3 pos);
    glm::vec3 GetPosition() const;
    glm::vec3 position;

protected:
private:
};

#endif  // INCLUDE_PLACEABLEOBJECT_H
