#ifndef TRANSFORM_HPP_INCLUDED
#define TRANSFORM_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Position uses world units; Euler angles use degrees in Z * Y * X order.
struct Transform {
    glm::vec3 position{0.0f};
    glm::vec3 rotationDegrees{0.0f};
    glm::vec3 scale{1.0f};

    [[nodiscard]] glm::mat4 RotationMatrix() const {
        return glm::rotate(glm::mat4(1.0f), glm::radians(rotationDegrees.z),
                           glm::vec3(0.0f, 0.0f, 1.0f)) *
               glm::rotate(glm::mat4(1.0f), glm::radians(rotationDegrees.y),
                           glm::vec3(0.0f, 1.0f, 0.0f)) *
               glm::rotate(glm::mat4(1.0f), glm::radians(rotationDegrees.x),
                           glm::vec3(1.0f, 0.0f, 0.0f));
    }

    [[nodiscard]] glm::mat4 ModelMatrix() const {
        return glm::translate(glm::mat4(1.0f), position) * RotationMatrix() *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

#endif  // TRANSFORM_HPP_INCLUDED
