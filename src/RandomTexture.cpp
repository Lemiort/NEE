#include "RandomTexture.hpp"

#include <glad/gl.h>

#include <vector>

#include "Math3d.hpp"
#include "Util.hpp"

RandomTexture::RandomTexture() : textureID(0) {
    // ctor
}

RandomTexture::~RandomTexture() {
    // dtor
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

bool RandomTexture::InitRandomTexture(unsigned int Size) {
    std::vector<Vector3f> pRandomData(Size);
    for (unsigned int i = 0; i < Size; i++) {
        pRandomData[i].x = RandomFloat();
        pRandomData[i].y = RandomFloat();
        pRandomData[i].z = RandomFloat();
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_1D, textureID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, Size, 0.0F, GL_RGB, GL_FLOAT,
                 pRandomData.data());
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    // vector cleans up automatically

    return GLCheckError();
}

void RandomTexture::Bind(GLenum TextureUnit) {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_1D, textureID);
}
