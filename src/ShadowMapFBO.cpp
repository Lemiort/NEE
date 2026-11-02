#include "ShadowMapFBO.hpp"

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>

#include "Texture.hpp"
#include "spdlog/spdlog.h"

ShadowMapFBO::ShadowMapFBO(GLsizei windowWidth, GLsizei windowHeight) {
    if (!Resize(windowWidth, windowHeight)) {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteTextures(1, &m_shadowMap);
        glDeleteTextures(static_cast<GLsizei>(m_textures.size()),
                         m_textures.data());
        throw std::runtime_error("Unable to create shadow-map framebuffer");
    }
}

ShadowMapFBO::~ShadowMapFBO() {
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }
    if (m_rbo != 0) {
        glDeleteRenderbuffers(1, &m_rbo);
    }

    if (m_shadowMap != 0) {
        glDeleteTextures(1, &m_shadowMap);
    }
    glDeleteTextures(static_cast<GLsizei>(m_textures.size()),
                     m_textures.data());
}

bool ShadowMapFBO::Resize(GLsizei windowWidth, GLsizei windowHeight) {
    if (windowWidth <= 0 || windowHeight <= 0) {
        spdlog::error("Shadow-map dimensions must be positive");
        return false;
    }
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteRenderbuffers(1, &m_rbo);
    glDeleteTextures(1, &m_shadowMap);
    glDeleteTextures(static_cast<GLsizei>(m_textures.size()),
                     m_textures.data());
    m_fbo = 0;
    m_rbo = 0;
    m_shadowMap = 0;
    m_textures.fill(0);
    glCreateTextures(GL_TEXTURE_2D, 1, &m_shadowMap);
    glTextureStorage2D(m_shadowMap, 1, GL_DEPTH_COMPONENT24, windowWidth,
                       windowHeight);
    glTextureParameteri(m_shadowMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_shadowMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_shadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_shadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateFramebuffers(1, &m_fbo);
    glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT, m_shadowMap, 0);
    glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_textures.size()),
                     m_textures.data());
    for (std::size_t i = 0; i < m_textures.size(); ++i) {
        const GLuint texture = m_textures.at(i);
        glTextureStorage2D(texture, 1, GL_RGBA32F, windowWidth, windowHeight);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glNamedFramebufferTexture(
            m_fbo, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), texture, 0);
    }

    const std::array<GLenum, 4> drawBuffers = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3};

    glNamedFramebufferDrawBuffers(
        m_fbo, static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    // check FBO status
    GLenum const status =
        glCheckNamedFramebufferStatus(m_fbo, GL_DRAW_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            spdlog::info("Framebuffer complete.");
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

void ShadowMapFBO::BindForWriting() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

GLuint ShadowMapFBO::GetTexture() const { return m_shadowMap; }

GLuint ShadowMapFBO::GetTexture(unsigned num) { return m_textures.at(num); }

std::string ShadowMapFBO::CheckShadowTexture() const {
    // Create a temporary object on the stack – no dynamic allocation needed
    Texture2D tempTexture(m_shadowMap, false);
    return tempTexture.GetParameters();
}

void ShadowMapFBO::BindForReading(GLenum TextureUnit) const {
    glBindTextureUnit(TextureUnit - GL_TEXTURE0, m_shadowMap);
}
