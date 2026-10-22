#ifndef TEXT_LAYOUT_HPP_INCLUDED
#define TEXT_LAYOUT_HPP_INCLUDED

#include <glm/ext/vector_float2.hpp>

// Screen-space text placement. Position is expressed in normalized device
// coordinates and size is the requested font height in pixels.
struct TextLayout {
    glm::vec2 position{-1.0F, 0.0F};
    float fontSizePixels{16.0F};
};

#endif  // TEXT_LAYOUT_HPP_INCLUDED
