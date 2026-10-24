#include "CubemapTexture.hpp"

#include <glad/gl.h>

#include <array>
#include <string>

#include "Texture.hpp"
CubemapTexture::CubemapTexture(const std::string& directory,
                               const std::string& positiveXFilename,
                               const std::string& negativeXFilename,
                               const std::string& positiveYFilename,
                               const std::string& negativeYFilename,
                               const std::string& positiveZFilename,
                               const std::string& negativeZFilename) {
    const std::string baseDirectory =
        directory.ends_with('/') ? directory : directory + "/";
    fileNames_ = {
        baseDirectory + positiveXFilename, baseDirectory + negativeXFilename,
        baseDirectory + positiveYFilename, baseDirectory + negativeYFilename,
        baseDirectory + positiveZFilename, baseDirectory + negativeZFilename};
}

CubemapTexture::~CubemapTexture() {
    if (textureObject_ != 0) {
        glDeleteTextures(1, &textureObject_);
    }
}

bool CubemapTexture::Load() {
    const GLuint replacement = loadCubeMap(fileNames_);
    if (replacement == 0) return false;
    if (textureObject_ != 0) glDeleteTextures(1, &textureObject_);
    textureObject_ = replacement;
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject_);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return true;
}

void CubemapTexture::Bind(GLenum TextureUnit) const {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject_);
}
