#include "RenderableObject.h"

std::shared_ptr<Shader> RenderableObject::GetShader() { return shaderProgram; }
void RenderableObject::SetShader(std::shared_ptr<Shader> shader) {
    shaderProgram = shader;
}
