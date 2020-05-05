#ifndef INCLUDE_RANDOMTEXTURE_H
#define INCLUDE_RANDOMTEXTURE_H
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "EngineCommon.h"

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

#endif  // INCLUDE_RANDOMTEXTURE_H
