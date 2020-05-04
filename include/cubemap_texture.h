#ifndef CUBEMAP_TEXTURE_H_INCLUDED
#define CUBEMAP_TEXTURE_H_INCLUDED
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

using namespace std;
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

#endif  // CUBEMAP_TEXTURE_H_INCLUDED
