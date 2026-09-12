#ifndef RANDOMTEXTURE_H
#define RANDOMTEXTURE_H
#include <glad/gl.h>

#include "EngineCommon.hpp"
#include "Math3d.hpp"
#include "Util.hpp"
class RandomTexture {
public:
    RandomTexture();
    virtual ~RandomTexture();
    bool InitRandomTexture(unsigned int Size);
    void Bind(GLenum TextureUnit);

protected:
private:
    GLuint textureID;
};

#endif  // RANDOMTEXTURE_H
