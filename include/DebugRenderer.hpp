#ifndef DEBUG_RENDERER_HPP_INCLUDED
#define DEBUG_RENDERER_HPP_INCLUDED

#include <glad/gl.h>

#include <memory>
#include <span>

#include "RenderQueue.hpp"

class Camera;
class Shader;

class DebugRenderer {
public:
    DebugRenderer() = default;
    ~DebugRenderer();

    DebugRenderer(const DebugRenderer&) = delete;
    DebugRenderer& operator=(const DebugRenderer&) = delete;

    [[nodiscard]] bool Init(std::shared_ptr<Shader> shader);
    void Render(std::span<const DebugLineRenderCommand> lines,
                const Camera& camera);

private:
    std::shared_ptr<Shader> shader_;
    GLuint vertexBuffer_{};
    GLint positionAttribute_{};
    GLint viewProjectionUniform_{};
    GLint colorUniform_{};
    GLint pointSizeUniform_{};
};

#endif  // DEBUG_RENDERER_HPP_INCLUDED
