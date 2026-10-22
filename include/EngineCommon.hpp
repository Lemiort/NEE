#ifndef ENGINECOMMON_H_INCLUDED
#define ENGINECOMMON_H_INCLUDED

#include <glad/gl.h>

inline constexpr GLenum COLOR_TEXTURE_UNIT = GL_TEXTURE0;
inline constexpr GLint COLOR_TEXTURE_UNIT_INDEX = 0;
inline constexpr GLenum SHADOW_TEXTURE_UNIT = GL_TEXTURE1;
inline constexpr GLint SHADOW_TEXTURE_UNIT_INDEX = 1;
inline constexpr GLenum NORMAL_TEXTURE_UNIT = GL_TEXTURE2;
inline constexpr GLint NORMAL_TEXTURE_UNIT_INDEX = 2;
inline constexpr GLenum RANDOM_TEXTURE_UNIT = GL_TEXTURE3;
inline constexpr GLint RANDOM_TEXTURE_UNIT_INDEX = 3;
inline constexpr GLint INVALID_UNIFORM_LOCATION = -1;

#endif  // ENGINECOMMON_H_INCLUDED
