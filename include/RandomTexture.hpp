#ifndef RANDOMTEXTURE_H
#define RANDOMTEXTURE_H
#include "EngineCommon.hpp"
#include "GL/glew.h"
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
