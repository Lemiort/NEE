#include "Assistant.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

const glm::mat4* Assistant::GetTSRVC() {
    transformation =
        glm::perspectiveRH_NO(glm::radians(persProj.FOV),
                              persProj.Width / persProj.Height, persProj.zNear,
                              persProj.zFar) *
        glm::lookAtRH(camera.Pos, camera.Pos + camera.Target, camera.Up) *
        GetModelTransform();
    return &transformation;
}

const glm::mat4* Assistant::GetTSR() {
    transformation = GetModelTransform();
    return &transformation;
}

const glm::mat4* Assistant::GetVC() {
    transformation =
        glm::perspectiveRH_NO(glm::radians(persProj.FOV),
                              persProj.Width / persProj.Height, persProj.zNear,
                              persProj.zFar) *
        glm::lookAtRH(camera.Pos, camera.Pos + camera.Target, camera.Up);
    return &transformation;
}

const glm::mat4* Assistant::GetRotate() {
    // Historically this also included translation and scale.
    transformation = GetModelTransform();
    return &transformation;
}

const glm::mat4* Assistant::GetScaleTrans() {
    transformation = glm::scale(glm::mat4(1.0f), scale);
    return &transformation;
}

const glm::mat4* Assistant::GetCameraTrans() {
    transformation = glm::translate(glm::mat4(1.0f), -camera.Pos);
    return &transformation;
}

glm::mat4 Assistant::GetModelTransform() const {
    // Keep the scene's Z * Y * X Euler order and degree based API.
    return glm::translate(glm::mat4(1.0f), worldPos) *
           glm::rotate(glm::mat4(1.0f), glm::radians(rotate.z), glm::vec3(0, 0, 1)) *
           glm::rotate(glm::mat4(1.0f), glm::radians(rotate.y), glm::vec3(0, 1, 0)) *
           glm::rotate(glm::mat4(1.0f), glm::radians(rotate.x), glm::vec3(1, 0, 0)) *
           glm::scale(glm::mat4(1.0f), scale);
}
