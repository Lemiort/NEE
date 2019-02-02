#ifndef RANDOMTEXTURE_H
#define RANDOMTEXTURE_H
#include "math_3d.h"
#include <EngineCommon.h>
#include <util.h>
#include "GL/glew.h"
class RandomTexture
{
    public:
        RandomTexture();
        virtual ~RandomTexture();
        bool InitRandomTexture(unsigned int Size);
        void Bind(GLenum TextureUnit);
    protected:
    private:
        GLuint textureID;
};

#endif // RANDOMTEXTURE_H


