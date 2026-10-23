#ifndef GBUFFER_H
#define GBUFFER_H

#include <glad/gl.h>

#include <array>

class GBuffer {
public:
    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_TEXTURE_TYPE_SPECULAR,
        GBUFFER_NUM_TEXTURES
    };

    GBuffer(GLsizei windowWidth, GLsizei windowHeight);
    ~GBuffer();

    bool Resize(GLsizei windowWidth, GLsizei windowHeight);

    void StartFrame();
    void BindForGeomPass() const;
    static void BindForStencilPass();
    void BindForLightPass();
    void BindForFinalPass();

    void BindForWriting() const;
    void BindForReading() const;
    static void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);

    void CheckTextures();

    GLuint GetTexture(unsigned num);

    [[nodiscard]] GLuint GetDepthTexture() const;

private:
    GLuint m_fbo{};
    std::array<GLuint, GBUFFER_NUM_TEXTURES> m_textures{};
    GLuint m_depthTexture{};
    GLuint m_testTexture{};
    GLuint m_finalTexture{};
};
#endif  // GBUFFER_H
