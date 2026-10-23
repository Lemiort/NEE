#ifndef TEXT_RENDERER_HPP_INCLUDED
#define TEXT_RENDERER_HPP_INCLUDED

#include <glad/gl.h>

#include <functional>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <unordered_map>

#include "Camera.hpp"
#include "RenderQueue.hpp"
#include "Shader.hpp"
#include "Text2D.hpp"
#include "Texture.hpp"

class TextRenderer {
public:
    explicit TextRenderer(gsl_lite::not_null<std::shared_ptr<Shader>> shader);
    ~TextRenderer();
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    [[nodiscard]] bool RegisterFont(const Font2d& font);
    void Render(const FontLine2d& line, const Camera& camera);
    void Render(std::span<const TextRenderCommand> commands,
                const Camera& camera);

private:
    void RenderBatch(
        std::span<const std::reference_wrapper<const FontLine2d>> lines,
        const Camera& camera);

    gsl_lite::not_null<std::shared_ptr<Shader>> shader_;
    std::unordered_map<std::string, std::unique_ptr<Texture2D>> atlases_;
    GLuint vertexBuffer_{};
    GLuint indexBuffer_{};
    GLint positionAttribute_{};
    GLint uvAttribute_{};
    GLint positionUniform_{};
    GLint uvUniform_{};
    GLint sizeUniform_{};
    GLint textureUniform_{};
    GLint colorUniform_{};
};

#endif  // TEXT_RENDERER_HPP_INCLUDED
