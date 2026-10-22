#include "RenderableObject.hpp"

#include <memory>
#include <utility>

#include "Camera.hpp"
#include "Shader.hpp"

RenderableObject::RenderableObject() = default;

RenderableObject::~RenderableObject() = default;

std::shared_ptr<Shader> RenderableObject::GetShader() const {
    return shaderProgram;
}
void RenderableObject::SetShader(std::shared_ptr<Shader> shader) {
    shaderProgram = std::move(shader);
}
