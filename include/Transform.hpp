#ifndef TRANSFORM_HPP_INCLUDED
#define TRANSFORM_HPP_INCLUDED

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>

// Position uses world units; Euler angles use degrees in Z * Y * X order.
struct Transform {
  glm::vec3 position{0.0F};
  glm::vec3 rotationDegrees{0.0F};
  glm::vec3 scale{1.0F};

  [[nodiscard]] glm::mat4 RotationMatrix() const {
    return glm::rotate(glm::mat4(1.0F), glm::radians(rotationDegrees.z),
                       glm::vec3(0.0F, 0.0F, 1.0F)) *
           glm::rotate(glm::mat4(1.0F), glm::radians(rotationDegrees.y),
                       glm::vec3(0.0F, 1.0F, 0.0F)) *
           glm::rotate(glm::mat4(1.0F), glm::radians(rotationDegrees.x),
                       glm::vec3(1.0F, 0.0F, 0.0F));
  }

    [[nodiscard]] glm::mat4 ModelMatrix() const {
      return glm::translate(glm::mat4(1.0F), position) * RotationMatrix() *
             glm::scale(glm::mat4(1.0F), scale);
    }
};

#endif  // TRANSFORM_HPP_INCLUDED
