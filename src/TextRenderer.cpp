#include "TextRenderer.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <stdexcept>
#include <utility>
#include <vector>

#include "spdlog/spdlog.h"

namespace {
const GLvoid* BufferOffset(std::size_t byteOffset) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,
    // performance-no-int-to-ptr)
    return reinterpret_cast<const GLvoid*>(
        static_cast<std::uintptr_t>(byteOffset));
}

struct TextVertex {
    glm::vec2 position;
    glm::vec2 uv;
};
}  // namespace

TextRenderer::~TextRenderer() {
    glDeleteBuffers(1, &vertexBuffer_);
    glDeleteBuffers(1, &indexBuffer_);
}

TextRenderer::TextRenderer(gsl_lite::not_null<std::shared_ptr<Shader>> shader)
    : shader_(std::move(shader)) {
    glGenBuffers(1, &vertexBuffer_);
    glGenBuffers(1, &indexBuffer_);

    if (vertexBuffer_ == 0 || indexBuffer_ == 0) {
        glDeleteBuffers(1, &vertexBuffer_);
        glDeleteBuffers(1, &indexBuffer_);
        spdlog::error("Unable to create text renderer buffers");
        throw std::runtime_error("Unable to create text renderer buffers");
    }

    positionAttribute_ = shader_->GetAttribLocation("Position");
    uvAttribute_ = shader_->GetAttribLocation("UV");
    positionUniform_ = shader_->GetUniformLocation("s_Position");
    uvUniform_ = shader_->GetUniformLocation("s_UV");
    sizeUniform_ = shader_->GetUniformLocation("size");
    textureUniform_ = shader_->GetUniformLocation("texSampler");
    colorUniform_ = shader_->GetUniformLocation("textColor");
}

bool TextRenderer::RegisterFont(const Font2d& font) {
    if (font.GetAtlasFilename().empty()) {
        return false;
    }
    if (atlases_.contains(font.GetAtlasFilename())) {
        return true;
    }

    auto texture = std::make_unique<Texture2D>();
    if (!texture->Load(font.GetAtlasFilename())) {
        return false;
    }
    atlases_.emplace(font.GetAtlasFilename(), std::move(texture));
    return true;
}

void TextRenderer::Render(const FontLine2d& line, const Camera& camera) {
    const std::array<std::reference_wrapper<const FontLine2d>, 1> lines{line};
    RenderBatch(lines, camera);
}

void TextRenderer::Render(std::span<const TextRenderCommand> commands,
                          const Camera& camera) {
    std::vector<std::reference_wrapper<const FontLine2d>> batch;
    for (const TextRenderCommand& command : commands) {
        const FontLine2d& line = command.text;
        if (!batch.empty() &&
            (batch.front().get().GetFont() != line.GetFont() ||
             batch.front().get().GetColor() != line.GetColor())) {
            RenderBatch(batch, camera);
            batch.clear();
        }
        batch.emplace_back(line);
    }
    RenderBatch(batch, camera);
}

void TextRenderer::RenderBatch(
    std::span<const std::reference_wrapper<const FontLine2d>> lines,
    const Camera& camera) {
    if (lines.empty()) {
        return;
    }
    const FontLine2d& firstLine = lines.front();
    const auto& font = firstLine.GetFont();
    if (font == nullptr) {
        return;
    }
    const auto atlas = atlases_.find(font->GetAtlasFilename());
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

    const FontCharacter& spacingGlyph = metrics.GetCharacter(34);
    const float spaceWidth = (2.0F * static_cast<float>(spacingGlyph.origW) /
                              static_cast<float>(viewportWidth)) /
                             static_cast<float>(metrics.GetFontHeight());
    std::vector<TextVertex> vertices;
    std::vector<uint32_t> indices;

    for (const FontLine2d& line : lines) {
        float cursorOffset = 0.0F;
        char16_t previous{};
        for (const char value : line.GetText()) {
            const auto current = static_cast<char16_t>(value);
            cursorOffset += metrics.GetKerning(previous, current);
            if (current == u' ') {
                cursorOffset += line.GetFontSize() * spaceWidth;
            }

            try {
                const FontCharacter& glyph = metrics.GetCharacter(current);
                const float fontScale =
                    line.GetFontSize() /
                    static_cast<float>(metrics.GetFontHeight());
                const float width = 2.0F * static_cast<float>(glyph.width) /
                                    static_cast<float>(viewportWidth) *
                                    fontScale;
                const float height = 2.0F * static_cast<float>(glyph.height) /
                                     static_cast<float>(viewportHeight) *
                                     fontScale;
                const float x = line.GetPosition().x + cursorOffset +
                                fontScale * 2.0F *
                                    static_cast<float>(glyph.xOffset) /
                                    static_cast<float>(viewportWidth);
                const float y = line.GetPosition().y +
                                fontScale * -2.0F *
                                    static_cast<float>(glyph.yOffset) /
                                    static_cast<float>(viewportHeight);
                const float uvX = static_cast<float>(glyph.xpos) /
                                  static_cast<float>(metrics.GetAtlasWidth());
                const float uvY =
                    1.0F - static_cast<float>(glyph.ypos) /
                               static_cast<float>(metrics.GetAtlasHeight());
                const float uvWidth =
                    static_cast<float>(glyph.width) /
                    static_cast<float>(metrics.GetAtlasWidth());
                const float uvHeight =
                    static_cast<float>(glyph.height) /
                    static_cast<float>(metrics.GetAtlasHeight());
                const auto baseVertex = static_cast<uint32_t>(vertices.size());
                vertices.insert(
                    vertices.end(),
                    {{{x, y - height}, {uvX, uvY - uvHeight}},
                     {{x, y}, {uvX, uvY}},
                     {{x + width, y - height}, {uvX + uvWidth, uvY - uvHeight}},
                     {{x + width, y}, {uvX + uvWidth, uvY}}});
                indices.insert(indices.end(), {baseVertex, baseVertex + 2,
                                               baseVertex + 1, baseVertex + 1,
                                               baseVertex + 2, baseVertex + 3});

                cursorOffset +=
                    width + (line.GetFontSize() * spaceWidth / 4.0F);
                previous = current;
            } catch (const std::out_of_range&) {
                // Unsupported characters are intentionally skipped.
            }
        }
    }
    if (vertices.empty()) {
        return;
    }

    shader_->Use();
    atlas->second->Bind(GL_TEXTURE0);
    glUniform1i(textureUniform_, 0);
    glUniform2f(positionUniform_, 0.0F, 0.0F);
    glUniform2f(uvUniform_, 0.0F, 0.0F);
    glUniform1f(sizeUniform_, 1.0F);
    const glm::vec4 color = firstLine.GetColor();
    glUniform4f(colorUniform_, color.r, color.g, color.b, color.a);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(TextVertex)),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(positionAttribute_, 2, GL_FLOAT, GL_FALSE,
                          sizeof(TextVertex), nullptr);
    glVertexAttribPointer(uvAttribute_, 2, GL_FLOAT, GL_FALSE,
                          sizeof(TextVertex),
                          BufferOffset(offsetof(TextVertex, uv)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
                 indices.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(positionAttribute_);
    glEnableVertexAttribArray(uvAttribute_);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()),
                   GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(positionAttribute_);
    glDisableVertexAttribArray(uvAttribute_);
}
