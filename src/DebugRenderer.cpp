#include "DebugRenderer.hpp"

#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

#include "Camera.hpp"
#include "Shader.hpp"

DebugRenderer::DebugRenderer(gsl_lite::not_null<std::shared_ptr<Shader>> shader)
    : shader_(std::move(shader)) {
    positionAttribute_ = shader_->GetAttribLocation("position");
    viewProjectionUniform_ = shader_->GetUniformLocation("gWVCP");
    colorUniform_ = shader_->GetUniformLocation("PixelColor");
    pointSizeUniform_ = shader_->GetUniformLocation("size");
    glCreateBuffers(1, &vertexBuffer_);
    glNamedBufferStorage(vertexBuffer_, 2 * sizeof(glm::vec3), nullptr,
                         GL_DYNAMIC_STORAGE_BIT);
    vertexArray_.SetBuffer(0, vertexBuffer_, sizeof(glm::vec3));
    vertexArray_.SetAttribute(positionAttribute_, 0, 3);
}

DebugRenderer::~DebugRenderer() {
    if (vertexBuffer_ != 0) {
        glDeleteBuffers(1, &vertexBuffer_);
    }
}

void DebugRenderer::Render(std::span<const DebugLineRenderCommand> lines,
                           const Camera& camera) {
    if (lines.empty()) {
        return;
    }

    shader_->Use();
    glUniformMatrix4fv(viewProjectionUniform_, 1, GL_FALSE,
                       glm::value_ptr(camera.GetViewProjectionMatrix()));
    glUniform1f(pointSizeUniform_, 1.5F);
    vertexArray_.Bind();

    for (const DebugLineRenderCommand& line : lines) {
        const std::array<glm::vec3, 2> vertices{line.start, line.end};
        glNamedBufferSubData(vertexBuffer_, 0, sizeof(vertices),
                             vertices.data());
        glUniform4f(colorUniform_, line.color.r, line.color.g, line.color.b,
                    1.0F);
        glDrawArrays(GL_LINES, 0, 2);
    }
}
