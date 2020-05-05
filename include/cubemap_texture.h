#ifndef INCLUDE_CUBEMAP_TEXTURE_H_
#define INCLUDE_CUBEMAP_TEXTURE_H_
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

using std::string;

class CubemapTexture {
public:
        CubemapTexture(const string& Directory, const string& PosXFilename,
                   const string& NegXFilename, const string& PosYFilename,
                   const string& NegYFilename, const string& PosZFilename,
                   const string& NegZFilename);

    ~CubemapTexture();

    bool Load();

    void Bind(GLenum TextureUnit);

private:
    string m_fileNames[6];

public:
    GLuint m_textureObj;
};

#endif  // INCLUDE_CUBEMAP_TEXTURE_H_
