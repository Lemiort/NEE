#include "RenderableObject.hpp"

#include <memory>
#include <utility>

#include "Camera.hpp"
#include "Shader.hpp"

RenderableObject::RenderableObject() {
    // ctor
}

RenderableObject::~RenderableObject() {
    // dtor
}

void RenderableObject::Render(const Camera& cam) {}

std::shared_ptr<Shader> RenderableObject::GetShader() { return shaderProgram; }
void RenderableObject::SetShader(std::shared_ptr<Shader> shader) {
    shaderProgram = std::move(shader);
}
