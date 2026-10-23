#ifndef CUBEMAP_TEXTURE_H_INCLUDED
#define CUBEMAP_TEXTURE_H_INCLUDED
#include <glad/gl.h>

#include <array>
#include <string>

class CubemapTexture {
public:
    CubemapTexture(const std::string& directory,
                   const std::string& positiveXFilename,
                   const std::string& negativeXFilename,
                   const std::string& positiveYFilename,
                   const std::string& negativeYFilename,
                   const std::string& positiveZFilename,
                   const std::string& negativeZFilename);

    ~CubemapTexture();

    bool Load();

    void Bind(GLenum TextureUnit) const;

private:
    std::array<std::string, 6> fileNames_;

    GLuint textureObject_{};
};

#endif  // CUBEMAP_TEXTURE_H_INCLUDED
