#include "TextRenderer.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace {
const GLvoid* BufferOffset(std::size_t byteOffset) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,
    // performance-no-int-to-ptr)
    return reinterpret_cast<const GLvoid*>(
        static_cast<std::uintptr_t>(byteOffset));
}
}  // namespace

TextRenderer::~TextRenderer() {
    glDeleteBuffers(1, &vertexBuffer_);
    glDeleteBuffers(1, &indexBuffer_);
}

bool TextRenderer::Init(std::shared_ptr<Shader> shader) {
    shader_ = std::move(shader);
    if (shader_ == nullptr) {
        return false;
    }

    glGenBuffers(1, &vertexBuffer_);
    glGenBuffers(1, &indexBuffer_);
    const std::array<uint32_t, 4> indices = {0, 2, 1, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                 GL_STATIC_DRAW);

    positionAttribute_ = shader_->GetAttribLocation("Position");
    uvAttribute_ = shader_->GetAttribLocation("UV");
    positionUniform_ = shader_->GetUniformLocation("s_Position");
    uvUniform_ = shader_->GetUniformLocation("s_UV");
    sizeUniform_ = shader_->GetUniformLocation("size");
    textureUniform_ = shader_->GetUniformLocation("texSampler");
    colorUniform_ = shader_->GetUniformLocation("textColor");
    return true;
}

bool TextRenderer::RegisterFont(const std::shared_ptr<const Font2d>& font) {
    if (font == nullptr || font->GetAtlasFilename().empty()) {
        return false;
    }
    if (atlases_.contains(font.get())) {
        return true;
    }

    auto texture = std::make_unique<Texture2D>();
    if (!texture->Load(font->GetAtlasFilename())) {
        return false;
    }
    atlases_.emplace(font.get(), std::move(texture));
    return true;
}

void TextRenderer::Render(const FontLine2d& line, const Camera& camera) {
    const auto& font = line.GetFont();
    if (shader_ == nullptr || font == nullptr) {
        return;
    }
    const auto atlas = atlases_.find(font.get());
    if (atlas == atlases_.end()) {
        return;
    }

    const FontMetrics& metrics = font->GetMetrics();
    const int viewportWidth = camera.GetWidth();
    const int viewportHeight = camera.GetHeight();
    if (viewportWidth <= 0 || viewportHeight <= 0 ||
        metrics.GetFontHeight() == 0 || metrics.GetAtlasWidth() == 0 ||
        metrics.GetAtlasHeight() == 0) {
        return;
    }

    shader_->Use();
    atlas->second->Bind(GL_TEXTURE0);
    glUniform1i(textureUniform_, 0);

    float cursorOffset = 0.0F;
    char16_t previous{};
    const FontCharacter& spacingGlyph = metrics.GetCharacter(34);
    const float spaceWidth = (2.0F * static_cast<float>(spacingGlyph.origW) /
                              static_cast<float>(viewportWidth)) /
                             static_cast<float>(metrics.GetFontHeight());

    for (const char value : line.GetText()) {
        const auto current = static_cast<char16_t>(value);
        cursorOffset += metrics.GetKerning(previous, current);
        if (current == u' ') {
            cursorOffset += line.GetFontSize() * spaceWidth;
        }

        try {
            const FontCharacter& glyph = metrics.GetCharacter(current);
            DrawGlyph(glyph, line.GetLayout(), line.GetColor(), metrics,
                      cursorOffset, viewportWidth, viewportHeight);

            const float fontScale = line.GetFontSize() /
                                    static_cast<float>(metrics.GetFontHeight());
            const float glyphWidth = 2.0F * static_cast<float>(glyph.width) /
                                     static_cast<float>(viewportWidth) *
                                     fontScale;
            cursorOffset +=
                glyphWidth + (line.GetFontSize() * spaceWidth / 4.0F);
            previous = current;
        } catch (const std::out_of_range&) {
            // Unsupported characters are intentionally skipped.
        }
    }
}

void TextRenderer::Render(std::span<const TextRenderCommand> commands,
                          const Camera& camera) {
    for (const TextRenderCommand& command : commands) {
        Render(command.text, camera);
    }
}

void TextRenderer::DrawGlyph(const FontCharacter& glyph,
                             const TextLayout& layout, const glm::vec4& color,
                             const FontMetrics& metrics, float cursorOffset,
                             int viewportWidth, int viewportHeight) {
    const float width = 2.0F * static_cast<float>(glyph.width) /
                        static_cast<float>(viewportWidth);
    const float height = 2.0F * static_cast<float>(glyph.height) /
                         static_cast<float>(viewportHeight);
    const std::array<float, 8> vertices = {0.0F,  -height, 0.0F,  0.0F,
                                           width, -height, width, 0.0F};

    const float uvWidth = static_cast<float>(glyph.width) /
                          static_cast<float>(metrics.GetAtlasWidth());
    const float uvHeight = static_cast<float>(glyph.height) /
                           static_cast<float>(metrics.GetAtlasHeight());
    const std::array<float, 8> uvs = {0.0F,    -uvHeight, 0.0F,    0.0F,
                                      uvWidth, -uvHeight, uvWidth, 0.0F};

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(uvs), nullptr,
                 GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(uvs), uvs.data());

    const float fontScale =
        layout.fontSizePixels / static_cast<float>(metrics.GetFontHeight());
    const float xOffset = fontScale * 2.0F * static_cast<float>(glyph.xOffset) /
                          static_cast<float>(viewportWidth);
    const float yOffset = fontScale * -2.0F *
                          static_cast<float>(glyph.yOffset) /
                          static_cast<float>(viewportHeight);
    glUniform2f(positionUniform_, layout.position.x + cursorOffset + xOffset,
                layout.position.y + yOffset);
    glUniform2f(uvUniform_,
                static_cast<float>(glyph.xpos) /
                    static_cast<float>(metrics.GetAtlasWidth()),
                1.0F - static_cast<float>(glyph.ypos) /
                           static_cast<float>(metrics.GetAtlasHeight()));
    glUniform1f(sizeUniform_, fontScale);
    glUniform4f(colorUniform_, color.r, color.g, color.b, color.a);

    glVertexAttribPointer(positionAttribute_, 2, GL_FLOAT, GL_FALSE, 0,
                          nullptr);
    glVertexAttribPointer(uvAttribute_, 2, GL_FLOAT, GL_FALSE, 0,
                          BufferOffset(sizeof(vertices)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
    glEnableVertexAttribArray(positionAttribute_);
    glEnableVertexAttribArray(uvAttribute_);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(positionAttribute_);
    glDisableVertexAttribArray(uvAttribute_);
}
