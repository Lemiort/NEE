#ifndef FONTFONT_H
#define FONTFONT_H

#include <map>
#include <string>

#include "Font/FontCharacter.h"

class Font {
public:
    Font() = default;
    void Init(std::string font_filename);
    virtual ~Font();

private:
    void AddCharacter(const std::string& line);
    void AddKerning(const std::string& line);
    void ParseName(const std::string& line);
    static std::string ParseImageFilename(const std::string& line);

    std::map<int, FontCharacter> font_map_;
    std::map<std::pair<int, int>, float> kerning_map_;
    /**
     * @brief size of font in pixels
     *
     */
    int size_;

    std::string name_;
};

#endif /* FONTFONT_H */
