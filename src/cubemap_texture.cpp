#include "cubemap_texture.h"

#include <stb_image.h>

#include <string>

static const GLenum types[6] = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
CubemapTexture::CubemapTexture(const std::string& filename)
    : filename_{filename} {
    m_textureObj = 0;
}

CubemapTexture::~CubemapTexture() {
    if (m_textureObj != 0) {
        glDeleteTextures(1, &m_textureObj);
    }
}

bool CubemapTexture::Load() {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* image =
        stbi_load(filename_.c_str(), &width, &height, &channels, 0);
    if (!image) {
        return false;
    }

    glGenTextures(1, &m_textureObj);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);

    const GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
    for (int face = 0; face < 6; ++face) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, format, width,
                     height, 0, format, GL_UNSIGNED_BYTE, image);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    stbi_image_free(image);

    return true;
}

void CubemapTexture::Bind(GLenum TextureUnit) {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);
}
