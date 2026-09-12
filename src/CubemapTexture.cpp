#include "CubemapTexture.hpp"

#include <glad/gl.h>

#include <cstdint>
#include <string>

#include "TgaLoader.hpp"
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

    for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(types); i++) {
        ASSERT(m_fileNames[i].c_str());

        TGAHeader const* header = nullptr;
        uint8_t* buffer = nullptr;
        uint32_t size = 0;
        GLint format = 0;
        GLint internalFormat = 0;
        //  GLuint    texture;
        // try to load image from file
        if (!LoadFile(m_fileNames[i].c_str(), true, &buffer, &size)) {
            return 0;
        }

        // if the file size is obviously smaller than the TGA header
        if (size <= sizeof(TGAHeader)) {
            // LOG_ERROR("Too small file \n", m_fileNames[i]);
            delete[] buffer;
            return 0;
        }

        header = reinterpret_cast<TGAHeader*>(buffer);

        // check the format of the TGA file - uncompressed RGB or RGBA image
        if (header->datatype != 2 ||
            (header->bitperpel != 24 && header->bitperpel != 32)) {
            // LOG_ERROR("Wrong TGA format '%s'\n", m_fileNames[i]);
            delete[] buffer;
            return 0;
        }

        // get the texture format
        format = (header->bitperpel == 24 ? GL_BGR : GL_BGRA);
        internalFormat = (format == GL_BGR ? GL_RGB8 : GL_RGBA8);

        glTexImage2D(types[i], 0, internalFormat, header->width, header->height,
                     0, format, GL_UNSIGNED_BYTE,
                     reinterpret_cast<const GLvoid*>(
                         buffer + sizeof(TGAHeader) + header->idlength));

        delete[] buffer;
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
