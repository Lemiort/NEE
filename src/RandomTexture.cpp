#include "RandomTexture.h"

#include <glm/gtc/random.hpp>

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

bool RandomTexture::InitRandomTexture(unsigned int size) {
    glm::vec3 pRandomData[size];
    for (unsigned int i = 0; i < size; i++) {
        pRandomData[i].x = glm::linearRand(0, 1);
        pRandomData[i].y = glm::linearRand(0, 1);
        pRandomData[i].z = glm::linearRand(0, 1);
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_1D, textureID);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0.0f, GL_RGB, GL_FLOAT,
                 pRandomData);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    return glGetError() == GL_NO_ERROR;
}

void RandomTexture::Bind(GLenum TextureUnit) {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_1D, textureID);
}
