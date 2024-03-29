#include "RandomTexture.h"

RandomTexture::RandomTexture() {
    // ctor
    textureID = 0;
}

RandomTexture::~RandomTexture() {
    // dtor
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

bool RandomTexture::InitRandomTexture(unsigned int Size) {
    Vector3f* pRandomData = new Vector3f[Size];
    for (unsigned int i = 0; i < Size; i++) {
        pRandomData[i].x = RandomFloat();
        pRandomData[i].y = RandomFloat();
        pRandomData[i].z = RandomFloat();
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_1D, textureID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, Size, 0.0f, GL_RGB, GL_FLOAT,
                 pRandomData);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    delete[] pRandomData;

    return GLCheckError();
}

void RandomTexture::Bind(GLenum TextureUnit) {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_1D, textureID);
}
