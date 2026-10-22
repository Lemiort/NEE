#ifndef SHADOWMAPFBO_H
#define SHADOWMAPFBO_H

#include <glad/gl.h>

#include <array>
#include <string>

class ShadowMapFBO {
public:
    ShadowMapFBO();
    virtual ~ShadowMapFBO();
    bool Init(GLsizei windowWidth, GLsizei windowHeight);

    [[nodiscard]] GLuint GetTexture() const;

    void BindForWriting() const;

    void BindForReading(GLenum TextureUnit) const;

    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };

    GLuint GetTexture(unsigned num);

    [[nodiscard]] std::string CheckShadowTexture() const;

   private:
    GLuint m_fbo{0};
    GLuint m_rbo{0};
    GLuint m_shadowMap{0};

    std::array<GLuint, GBUFFER_NUM_TEXTURES> m_textures{};
};

#endif  // SHADOWMAPFBO_H
