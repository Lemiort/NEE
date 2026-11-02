#include "RandomTexture.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <random>
#include <vector>

#include "Util.hpp"

RandomTexture::RandomTexture() = default;

RandomTexture::~RandomTexture() {
    // dtor
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

bool RandomTexture::InitRandomTexture(GLsizei size) {
    if (size <= 0) {
        return false;
    }
    std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(0.0F, 1.0F);
    const auto randomFloat = [&generator, &distribution] {
        return distribution(generator);
    };
    std::vector<glm::vec3> pRandomData(static_cast<std::size_t>(size));
    for (GLsizei i = 0; i < size; ++i) {
        auto& randomVector = pRandomData.at(static_cast<std::size_t>(i));
        randomVector.x = randomFloat();
        randomVector.y = randomFloat();
        randomVector.z = randomFloat();
    }

    if (textureID != 0) glDeleteTextures(1, &textureID);
    glCreateTextures(GL_TEXTURE_1D, 1, &textureID);
    glTextureStorage1D(textureID, 1, GL_RGB32F, size);
    glTextureSubImage1D(textureID, 0, 0, size, GL_RGB, GL_FLOAT,
                        pRandomData.data());
    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);

    // vector cleans up automatically

    return GLCheckError();
}

void RandomTexture::Bind(GLenum TextureUnit) const {
    glBindTextureUnit(TextureUnit - GL_TEXTURE0, textureID);
}
