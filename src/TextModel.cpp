#include <utility>

#include "Text2D.hpp"

void FontLine2d::SetFont(std::shared_ptr<const Font2d> font) noexcept {
    font_ = std::move(font);
}

const std::shared_ptr<const Font2d>& FontLine2d::GetFont() const noexcept {
    return font_;
}

void FontLine2d::SetText(std::string text) { text_ = std::move(text); }

std::string_view FontLine2d::GetText() const noexcept { return text_; }

void FontLine2d::SetPosition(float x, float y) noexcept {
    SetPosition(glm::vec2{x, y});
}

void FontLine2d::SetPosition(const glm::vec2& position) noexcept {
    layout_.position = position;
}

const glm::vec2& FontLine2d::GetPosition() const noexcept {
    return layout_.position;
}

void FontLine2d::SetFontSize(float fontSizePixels) noexcept {
    layout_.fontSizePixels = fontSizePixels;
}

float FontLine2d::GetFontSize() const noexcept {
    return layout_.fontSizePixels;
}

void FontLine2d::SetColor(const glm::vec4& color) noexcept { color_ = color; }

const glm::vec4& FontLine2d::GetColor() const noexcept { return color_; }

const TextLayout& FontLine2d::GetLayout() const noexcept { return layout_; }
