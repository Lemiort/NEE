#ifndef TEXT_2D_H_INCLUDED
#define TEXT_2D_H_INCLUDED

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <memory>
#include <string>
#include <string_view>

#include "FontMetrics.hpp"
#include "TextLayout.hpp"

// CPU-side font asset. GPU resources are created and owned by TextRenderer.
class Font2d {
public:
    [[nodiscard]] bool Load(std::string filename);
    [[nodiscard]] const FontMetrics& GetMetrics() const noexcept;
    [[nodiscard]] const std::string& GetAtlasFilename() const noexcept;

private:
    FontMetrics metrics_;
    std::string atlasFilename_;
};

// A text instance contains presentation data only
class FontLine2d {
public:
    void SetFont(std::shared_ptr<const Font2d> font) noexcept;
    [[nodiscard]] const std::shared_ptr<const Font2d>& GetFont() const noexcept;
    void SetText(std::string text);
    [[nodiscard]] std::string_view GetText() const noexcept;
    void SetPosition(float x, float y) noexcept;
    void SetPosition(const glm::vec2& position) noexcept;
    [[nodiscard]] const glm::vec2& GetPosition() const noexcept;
    void SetFontSize(float fontSizePixels) noexcept;
    [[nodiscard]] float GetFontSize() const noexcept;
    void SetColor(const glm::vec4& color) noexcept;
    [[nodiscard]] const glm::vec4& GetColor() const noexcept;
    [[nodiscard]] const TextLayout& GetLayout() const noexcept;

private:
    std::shared_ptr<const Font2d> font_;
    std::string text_;
    TextLayout layout_{};
    glm::vec4 color_{1.0F};
};

#endif  // TEXT_2D_H_INCLUDED
