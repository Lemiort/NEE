#include "Transformable.hpp"

const Transform& Transformable::GetTransform() const noexcept {
    return transform_;
}

void Transformable::SetTransform(const Transform& transform) noexcept {
    transform_ = transform;
}

void Transformable::SetPosition(float x, float y, float z) noexcept {
    SetPosition(glm::vec3{x, y, z});
}

void Transformable::SetPosition(const glm::vec3& position) noexcept {
    transform_.position = position;
}

const glm::vec3& Transformable::GetPosition() const noexcept {
    return transform_.position;
}

void Transformable::SetRotation(float x, float y, float z) noexcept {
    SetRotation(glm::vec3{x, y, z});
}

void Transformable::SetRotation(const glm::vec3& rotationDegrees) noexcept {
    transform_.rotationDegrees = rotationDegrees;
}

const glm::vec3& Transformable::GetRotation() const noexcept {
    return transform_.rotationDegrees;
}

void Transformable::SetScale(float x, float y, float z) noexcept {
    SetScale(glm::vec3{x, y, z});
}

void Transformable::SetScale(const glm::vec3& scale) noexcept {
    transform_.scale = scale;
}

const glm::vec3& Transformable::GetScale() const noexcept {
    return transform_.scale;
}
