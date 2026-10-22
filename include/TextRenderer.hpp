#ifndef TEXT_RENDERER_HPP_INCLUDED
#define TEXT_RENDERER_HPP_INCLUDED

#include <glad/gl.h>

#include <memory>
#include <unordered_map>

#include "Camera.hpp"
#include "Shader.hpp"
#include "Text2D.hpp"
#include "Texture.hpp"

class TextRenderer {
public:
    TextRenderer() = default;
    ~TextRenderer();
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    [[nodiscard]] bool Init(std::shared_ptr<Shader> shader);
    [[nodiscard]] bool RegisterFont(const std::shared_ptr<const Font2d>& font);
    void Render(const FontLine2d& line, const Camera& camera);

private:
    void DrawGlyph(const FontCharacter& glyph, const TextLayout& layout,
                   const glm::vec4& color, const FontMetrics& metrics,
                   float cursorOffset, int viewportWidth, int viewportHeight);

    std::shared_ptr<Shader> shader_;
    std::unordered_map<const Font2d*, std::unique_ptr<Texture2D>> atlases_;
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
