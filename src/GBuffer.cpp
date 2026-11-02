#include "GBuffer.hpp"

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>

#include "Texture.hpp"
#include "spdlog/spdlog.h"

GBuffer::GBuffer(GLsizei windowWidth, GLsizei windowHeight) {
    if (!Resize(windowWidth, windowHeight)) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteTextures(static_cast<GLsizei>(m_textures.size()),
                         m_textures.data());
        glDeleteTextures(1, &m_depthTexture);
        glDeleteTextures(1, &m_testTexture);
        glDeleteTextures(1, &m_finalTexture);
        throw std::runtime_error("Unable to create G-buffer");
    }
}

GBuffer::~GBuffer() {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(static_cast<GLsizei>(m_textures.size()),
                     m_textures.data());
    glDeleteTextures(1, &m_depthTexture);
    glDeleteTextures(1, &m_testTexture);
    glDeleteTextures(1, &m_finalTexture);
}

bool GBuffer::Resize(GLsizei windowWidth, GLsizei windowHeight) {
    if (windowWidth <= 0 || windowHeight <= 0) {
        spdlog::error("G-buffer dimensions must be positive");
        return false;
    }
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(static_cast<GLsizei>(m_textures.size()),
                     m_textures.data());
    glDeleteTextures(1, &m_depthTexture);
    glDeleteTextures(1, &m_testTexture);
    glDeleteTextures(1, &m_finalTexture);
    m_fbo = 0;
    m_textures.fill(0);
    m_depthTexture = 0;
    m_testTexture = 0;
    m_finalTexture = 0;
    glCreateFramebuffers(1, &m_fbo);
    glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_textures.size()),
                     m_textures.data());
    for (std::size_t i = 0; i < m_textures.size(); ++i) {
        const GLuint texture = m_textures.at(i);
        glTextureStorage2D(texture, 1, GL_RGBA32F, windowWidth, windowHeight);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glNamedFramebufferTexture(
            m_fbo, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), texture, 0);
    }
    glCreateTextures(GL_TEXTURE_2D, 1, &m_depthTexture);
    glTextureStorage2D(m_depthTexture, 1, GL_DEPTH32F_STENCIL8, windowWidth,
                       windowHeight);
    glNamedFramebufferTexture(m_fbo, GL_DEPTH_STENCIL_ATTACHMENT,
                              m_depthTexture, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_finalTexture);
    glTextureStorage2D(m_finalTexture, 1, GL_RGBA8, windowWidth, windowHeight);
    glTextureParameteri(m_finalTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_finalTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNamedFramebufferTexture(
        m_fbo, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_textures.size()),
        m_finalTexture, 0);

    const std::array<GLenum, 5> drawBuffers = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};

    glNamedFramebufferDrawBuffers(
        m_fbo, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    spdlog::info("draw buffers: {}", drawBuffers.size());

    // check FBO status
    GLenum const status =
        glCheckNamedFramebufferStatus(m_fbo, GL_DRAW_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            spdlog::info("Framebuffer (GBuffer) complete.");
            return true;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            spdlog::error(
                "Framebuffer incomplete: Attachment is NOT complete.");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            spdlog::error(
                "Framebuffer incomplete: No image is attached to FBO.");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            spdlog::error("Framebuffer incomplete: Draw buffer.");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            spdlog::error("Framebuffer incomplete: Read buffer.");
            return false;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            spdlog::error(
                "Framebuffer incomplete: Unsupported by FBO implementation.");
            return false;

        default:
            spdlog::error("Framebuffer incomplete: Unknown error.");
            return false;
    }
}

void GBuffer::BindForWriting() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void GBuffer::BindForReading() const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
}

void GBuffer::SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType) {
    glReadBuffer(GL_COLOR_ATTACHMENT0 + TextureType);
}

void GBuffer::CheckTextures() {
    for (const GLuint m_texture : m_textures) {
        auto temp = std::make_unique<Texture2D>(m_texture, false);
        spdlog::info("{}", temp->GetParameters());
    }
}

GLuint GBuffer::GetTexture(unsigned num) { return m_textures.at(num); }

GLuint GBuffer::GetDepthTexture() const { return m_depthTexture; }

void GBuffer::StartFrame() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_textures.size()));
    glClear(GL_COLOR_BUFFER_BIT);
}

void GBuffer::BindForGeomPass() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    const std::array<GLenum, 5> drawBuffers = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};

    glNamedFramebufferDrawBuffers(
        m_fbo, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
}

void GBuffer::BindForStencilPass() {
    // should disable color buffer
    glDrawBuffer(GL_NONE);
}

void GBuffer::BindForLightPass() {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_textures.size()));
}

void GBuffer::BindForFinalPass() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_textures.size()));
}
