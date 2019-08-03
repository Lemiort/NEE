#include "RenderableObject.h"

RenderableObject::RenderableObject() {
    // ctor
}

RenderableObject::~RenderableObject() {
    // dtor
}

void RenderableObject::Render(Camera* cam) {}

shared_ptr<Shader> RenderableObject::GetShader() { return shaderProgram; }
void RenderableObject::SetShader(shared_ptr<Shader> shader) {
    shaderProgram = shader;
}
