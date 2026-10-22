#ifndef TRANSFORMABLE_HPP_INCLUDED
#define TRANSFORMABLE_HPP_INCLUDED

#include <glm/ext/vector_float3.hpp>

#include "Transform.hpp"

// Interface for scene objects whose complete spatial transform can be changed.
class ITransformable {
public:
    virtual ~ITransformable() = default;

    [[nodiscard]] virtual const Transform& GetTransform() const noexcept = 0;
    virtual void SetTransform(const Transform& transform) noexcept = 0;
};

// Reusable transform storage. Position, rotation and scale form one semantic
// unit and therefore cannot get split across independent base subobjects.
class Transformable : public ITransformable {
public:
    [[nodiscard]] const Transform& GetTransform() const noexcept override;
    void SetTransform(const Transform& transform) noexcept override;

    void SetPosition(float x, float y, float z) noexcept;
    void SetPosition(const glm::vec3& position) noexcept;
    [[nodiscard]] const glm::vec3& GetPosition() const noexcept;

    void SetRotation(float x, float y, float z) noexcept;
    void SetRotation(const glm::vec3& rotationDegrees) noexcept;
    [[nodiscard]] const glm::vec3& GetRotation() const noexcept;

    void SetScale(float x, float y, float z) noexcept;
    void SetScale(const glm::vec3& scale) noexcept;
    [[nodiscard]] const glm::vec3& GetScale() const noexcept;

protected:
    Transform transform_{};
};

#endif  // TRANSFORMABLE_HPP_INCLUDED
