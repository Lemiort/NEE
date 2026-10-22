#include "FontMetrics.hpp"

#include <utility>

void FontMetrics::SetFont(std::string name, uint32_t height) {
    fontName_ = std::move(name);
    fontHeight_ = height;
}

void FontMetrics::SetAtlasSize(uint32_t width, uint32_t height) noexcept {
    atlasWidth_ = width;
    atlasHeight_ = height;
}

void FontMetrics::AddCharacter(char16_t code, const FontCharacter& character) {
    characters_.insert_or_assign(code, character);
}

void FontMetrics::AddKerning(char16_t previous, char16_t current,
                             float amount) {
    kerning_.insert_or_assign({previous, current}, amount);
}

const FontCharacter& FontMetrics::GetCharacter(char16_t code) const {
    return characters_.at(code);
}

float FontMetrics::GetKerning(char16_t previous,
                              char16_t current) const noexcept {
    const auto found = kerning_.find({previous, current});
    return found == kerning_.end() ? 0.0F : found->second;
}

const std::string& FontMetrics::GetFontName() const noexcept {
    return fontName_;
}

uint32_t FontMetrics::GetFontHeight() const noexcept { return fontHeight_; }

uint32_t FontMetrics::GetAtlasWidth() const noexcept { return atlasWidth_; }

uint32_t FontMetrics::GetAtlasHeight() const noexcept { return atlasHeight_; }
