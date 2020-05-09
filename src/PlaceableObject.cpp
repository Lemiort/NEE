#include "PlaceableObject.h"

void PlaceableObject::SetPosition(glm::vec3 pos) { position = pos; }

glm::vec3 PlaceableObject::GetPosition() const { return position; }
