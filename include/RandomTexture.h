#ifndef INCLUDE_RANDOMTEXTURE_H_
#define INCLUDE_RANDOMTEXTURE_H_
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "EngineCommon.h"
#include "util.h"
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

#endif  // INCLUDE_RANDOMTEXTURE_H_
