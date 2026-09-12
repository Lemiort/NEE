#include "CubemapTexture.hpp"

#include <glad/gl.h>

#include <array>
#include <cstdint>
#include <string>

#include "Texture.hpp"
#include "Util.hpp"
static const GLenum types[6] = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
CubemapTexture::CubemapTexture(const string& Directory,
                               const string& PosXFilename,
                               const string& NegXFilename,
                               const string& PosYFilename,
                               const string& NegYFilename,
                               const string& PosZFilename,
                               const string& NegZFilename) {
    string::const_iterator it = Directory.end();
    it--;
    string const BaseDir = (*it == '/') ? Directory : Directory + "/";

    m_fileNames[0] = BaseDir + PosXFilename;
    m_fileNames[1] = BaseDir + NegXFilename;
    m_fileNames[2] = BaseDir + PosYFilename;
    m_fileNames[3] = BaseDir + NegYFilename;
    m_fileNames[4] = BaseDir + PosZFilename;
    m_fileNames[5] = BaseDir + NegZFilename;
}

CubemapTexture::~CubemapTexture() {
    if (m_textureObj != 0) {
        glDeleteTextures(1, &m_textureObj);
    }
}

bool CubemapTexture::Load() {
    glGenTextures(1, &m_textureObj);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);

    // Magick::Image* pImage = NULL;
    // Magick::Blob blob;

    // Load the cubemap using the helper from Texture.cpp
    std::array<std::string, 6> faces = {m_fileNames[0], m_fileNames[1],
                                        m_fileNames[2], m_fileNames[3],
                                        m_fileNames[4], m_fileNames[5]};

    m_textureObj = loadCubeMap(faces);
    if (m_textureObj == 0) {
        return false;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return true;
}

void CubemapTexture::Bind(GLenum TextureUnit) {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);
}
