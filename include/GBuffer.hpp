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
        GBUFFER_TEXTURE_TYPE_EMISSIVE,
        GBUFFER_NUM_TEXTURES
    };

    GBuffer(GLsizei windowWidth, GLsizei windowHeight);
    ~GBuffer();
    GBuffer(const GBuffer&) = delete;
    GBuffer& operator=(const GBuffer&) = delete;
    GBuffer(GBuffer&&) = delete;
    GBuffer& operator=(GBuffer&&) = delete;

    bool Resize(GLsizei windowWidth, GLsizei windowHeight);

    void StartFrame();
    void BindForGeomPass() const;
    static void BindForStencilPass();
    void BindForLightPass();
    void BindForFinalPass();
    // Starts the light accumulation target with the emissive color of the
    // geometry pass, so lights then add on top of what surfaces give off.
    void CopyEmissiveToLight() const;

    void BindForWriting() const;
    void BindForReading() const;
    static void SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType);

    void CheckTextures();

    GLuint GetTexture(unsigned num);

    [[nodiscard]] GLuint GetDepthTexture() const;
    // The light accumulation target, in half floats so light can pass 1.
    [[nodiscard]] GLuint GetLightTexture() const;

private:
    GLuint m_fbo{};
    std::array<GLuint, GBUFFER_NUM_TEXTURES> m_textures{};
    GLuint m_depthTexture{};
    GLuint m_testTexture{};
    GLuint m_finalTexture{};
    GLsizei m_width{};
    GLsizei m_height{};
};
#endif  // GBUFFER_H
