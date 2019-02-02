#ifndef CANERA_CAMERA_H
#define CANERA_CAMERA_H

// Include all GLM core / GLSL features
#include <glm/glm.hpp>  // vec2, vec3, mat4, radians

// Include all GLM extensions
#include <glm/ext.hpp>  // perspective, translate, rotate

#define GLM_FORCE_MESSAGES

class Camera {
public:
    Camera();
    virtual ~Camera() = default;

private:
    glm::vec3 position_;
    glm::vec3 target_;
};

#endif /* CANERA_CAMERA_H */
