#ifndef FONT_METRICS_HPP_INCLUDED
#define FONT_METRICS_HPP_INCLUDED

#include <cstdint>
#include <map>
#include <string>
#include <utility>

struct FontCharacter {
    uint32_t xpos{};
    uint32_t ypos{};
    uint32_t width{};
    uint32_t height{};
    int32_t xOffset{};
    int32_t yOffset{};
    uint32_t origW{};
    uint32_t origH{};
};

using KerningPair = std::pair<char16_t, char16_t>;

class FontMetrics {
public:
    void SetFont(std::string name, uint32_t height);
    void SetAtlasSize(uint32_t width, uint32_t height) noexcept;
    void AddCharacter(char16_t code, const FontCharacter& character);
    void AddKerning(char16_t previous, char16_t current, float amount);

    [[nodiscard]] const FontCharacter& GetCharacter(char16_t code) const;
    [[nodiscard]] float GetKerning(char16_t previous,
                                   char16_t current) const noexcept;
    [[nodiscard]] const std::string& GetFontName() const noexcept;
    [[nodiscard]] uint32_t GetFontHeight() const noexcept;
    [[nodiscard]] uint32_t GetAtlasWidth() const noexcept;
    [[nodiscard]] uint32_t GetAtlasHeight() const noexcept;

private:
    std::string fontName_;
    uint32_t fontHeight_{};
    uint32_t atlasWidth_{};
    uint32_t atlasHeight_{};
    std::map<char16_t, FontCharacter> characters_;
    std::map<KerningPair, float> kerning_;
};

#endif  // FONT_METRICS_HPP_INCLUDED
