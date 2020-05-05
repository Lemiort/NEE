#ifndef INCLUDE_CUBEMAP_TEXTURE_H
#define INCLUDE_CUBEMAP_TEXTURE_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

class CubemapTexture {
public:
    CubemapTexture(const std::string& filename);

    ~CubemapTexture();

    bool Load();

    void Bind(GLenum TextureUnit);

private:
    std::string filename_;

public:
    GLuint m_textureObj;
};

#endif  // INCLUDE_CUBEMAP_TEXTURE_H
