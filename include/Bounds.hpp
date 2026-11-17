#ifndef BOUNDS_HPP_INCLUDED
#define BOUNDS_HPP_INCLUDED

#include <array>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

// An axis-aligned box. A default box is empty: it contains nothing, and
// growing it by a point makes it that point.
struct Bounds {
    glm::vec3 min{1.0F};
    glm::vec3 max{-1.0F};

    [[nodiscard]] bool IsEmpty() const noexcept {
        return min.x > max.x || min.y > max.y || min.z > max.z;
    }
    void Grow(const glm::vec3& point) noexcept;
    void Grow(const Bounds& other) noexcept;
    // The box around this box after `matrix` moves its eight corners.
    [[nodiscard]] Bounds Transformed(const glm::mat4& matrix) const noexcept;
};

// The six planes of a camera's view volume, for culling what it can't see.
class Frustum {
public:
    // From an OpenGL view-projection matrix (clip depth -1 to 1).
    [[nodiscard]] static Frustum FromViewProjection(
        const glm::mat4& viewProjection) noexcept;
    // False only when the box is certainly outside; a box near a corner of
    // the view volume may count as inside. Empty boxes are outside.
    [[nodiscard]] bool Intersects(const Bounds& bounds) const noexcept;

private:
    // Each plane as (normal, distance), the normal pointing inward.
    std::array<glm::vec4, 6> planes_{};
};

#endif  // BOUNDS_HPP_INCLUDED
