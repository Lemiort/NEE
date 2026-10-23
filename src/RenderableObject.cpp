#include "RenderableObject.hpp"

#include <memory>
#include <utility>

#include "Camera.hpp"
#include "Shader.hpp"

RenderableObject::RenderableObject() = default;

RenderableObject::RenderableObject(RenderableObject&& other) noexcept
    : shaderProgram(std::move(other.shaderProgram)),
      VBO(std::exchange(other.VBO, 0)),
      IBO(std::exchange(other.IBO, 0)) {}

RenderableObject& RenderableObject::operator=(
    RenderableObject&& other) noexcept {
    if (this != &other) {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &IBO);
        shaderProgram = std::move(other.shaderProgram);
        VBO = std::exchange(other.VBO, 0);
        IBO = std::exchange(other.IBO, 0);
    }
    return *this;
}

RenderableObject::~RenderableObject() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}

std::shared_ptr<Shader> RenderableObject::GetShader() const {
    return shaderProgram;
}
void RenderableObject::SetShader(std::shared_ptr<Shader> shader) {
    shaderProgram = std::move(shader);
}
