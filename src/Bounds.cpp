#include "Bounds.hpp"

#include <glm/common.hpp>
#include <glm/geometric.hpp>

void Bounds::Grow(const glm::vec3& point) noexcept {
    if (IsEmpty()) {
        min = point;
        max = point;
        return;
    }
    min = glm::min(min, point);
    max = glm::max(max, point);
}

void Bounds::Grow(const Bounds& other) noexcept {
    if (!other.IsEmpty()) {
        Grow(other.min);
        Grow(other.max);
    }
}

Bounds Bounds::Transformed(const glm::mat4& matrix) const noexcept {
    Bounds result;
    if (IsEmpty()) {
        return result;
    }
    for (int corner = 0; corner < 8; ++corner) {
        const glm::vec3 point{(corner & 1) != 0 ? max.x : min.x,
                              (corner & 2) != 0 ? max.y : min.y,
                              (corner & 4) != 0 ? max.z : min.z};
        result.Grow(glm::vec3(matrix * glm::vec4(point, 1.0F)));
    }
    return result;
}

Frustum Frustum::FromViewProjection(const glm::mat4& viewProjection) noexcept {
    // Gribb and Hartmann: each plane is the last row of the matrix plus or
    // minus one of the others. glm matrices are column-major, so row i is
    // (m[0][i], m[1][i], m[2][i], m[3][i]).
    const auto row = [&viewProjection](int index) {
        return glm::vec4(viewProjection[0][index], viewProjection[1][index],
                         viewProjection[2][index], viewProjection[3][index]);
    };
    Frustum frustum;
    frustum.planes_ = {row(3) + row(0), row(3) - row(0), row(3) + row(1),
                       row(3) - row(1), row(3) + row(2), row(3) - row(2)};
    for (glm::vec4& plane : frustum.planes_) {
        const float length = glm::length(glm::vec3(plane));
        if (length > 0.0F) {
            plane /= length;
        }
    }
    return frustum;
}

bool Frustum::Intersects(const Bounds& bounds) const noexcept {
    if (bounds.IsEmpty()) {
        return false;
    }
    for (const glm::vec4& plane : planes_) {
        // The corner farthest along the plane's normal.
        const glm::vec3 corner{plane.x >= 0.0F ? bounds.max.x : bounds.min.x,
                               plane.y >= 0.0F ? bounds.max.y : bounds.min.y,
                               plane.z >= 0.0F ? bounds.max.z : bounds.min.z};
        if (glm::dot(glm::vec3(plane), corner) + plane.w < 0.0F) {
            return false;
        }
    }
    return true;
}
