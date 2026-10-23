#include "Text2D.hpp"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#include "spdlog/spdlog.h"
#include "stb_image.h"

bool Font2d::Load(std::string filename) {
    std::fstream input(filename, std::ios::in);
    if (!input) {
        spdlog::error("Unable to open font description '{}'", filename);
        return false;
    }

    enum class FontFileState { None, FontData, KerningData };
    FontFileState state = FontFileState::None;
    std::string line;
    while (std::getline(input, line)) {
        if (state == FontFileState::FontData) {
            std::stringstream stream(line);
            uint16_t code{};
            FontCharacter character{};
            if (stream >> code >> character.xpos >> character.ypos >>
                character.width >> character.height >> character.xOffset >>
                character.yOffset >> character.origW >> character.origH) {
                metrics_.AddCharacter(static_cast<char16_t>(code), character);
            }
        } else if (state == FontFileState::KerningData) {
            std::stringstream stream(line);
            uint16_t previous{};
            uint16_t current{};
            float amount = NAN;
            if (stream >> previous >> current >> amount) {
                metrics_.AddKerning(static_cast<char16_t>(previous),
                                    static_cast<char16_t>(current), amount);
            }
        }

        std::smatch match;
        const std::regex texturePattern(R"(textures:\s*([a-zA-Z0-9_\-\.]+))");
        if (std::regex_search(line, match, texturePattern)) {
            atlasFilename_ = std::string("fonts/") + match.str(1);
        }
        const std::regex fontPattern(R"((.*?)\s+(\d+)px$)");
        if (std::regex_search(line, match, fontPattern)) {
            metrics_.SetFont(match.str(1),
                             static_cast<uint32_t>(std::stoul(match.str(2))));
            state = FontFileState::FontData;
        }
        if (line.contains("kerning pairs:")) {
            state = FontFileState::KerningData;
        }
    }

    int width{};
    int height{};
    int channels{};
    if (atlasFilename_.empty() ||
        stbi_info(atlasFilename_.c_str(), &width, &height, &channels) == 0) {
        spdlog::error("Error reading font atlas '{}': {}", atlasFilename_,
                      stbi_failure_reason());
        return false;
    }
    metrics_.SetAtlasSize(static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height));
    return metrics_.GetFontHeight() != 0;
}

const FontMetrics& Font2d::GetMetrics() const noexcept { return metrics_; }
const std::string& Font2d::GetAtlasFilename() const noexcept {
    return atlasFilename_;
}
