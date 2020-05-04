#ifndef SHADOWMAPFBO_H
#define SHADOWMAPFBO_H

#include <GL/glew.h>

#include <string>

class ShadowMapFBO {
public:
    ShadowMapFBO();
    virtual ~ShadowMapFBO();
    bool Init(unsigned int WindowWidth, unsigned int WindowHeight);

    GLuint GetTexture();

    void BindForWriting();

    void BindForReading(GLenum TextureUnit);

    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };

    GLuint GetTexture(unsigned num);

    std::string CheckShadowTexture();

private:
    GLuint m_fbo;
    GLuint m_rbo;
    GLuint m_shadowMap;

    GLuint m_textures[GBUFFER_NUM_TEXTURES];
};

#endif  // SHADOWMAPFBO_H
