#ifndef FONTFONTCHARACTER_H
#define FONTFONTCHARACTER_H
#include <cstdint>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>  // vec2, vec3, mat4, radians

/**
 * @brief Represents character on font mtexture
 *
 */
struct FontCharacter {
    //    X pos   Y pos   Width   Height   Xoffset  Yoffset  Orig W   Orig H

    // offset on texture in px
    unsigned x_pos, y_pos;
    // width and height of actual symbol
    unsigned figure_width, figure_height;
    unsigned character_width, character_height;
    int x_offset, y_offset;
    int code;
};

#endif /* FONTFONTCHARACTER_H */
