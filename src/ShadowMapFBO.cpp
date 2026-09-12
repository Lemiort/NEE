#include "ShadowMapFBO.hpp"

#include <glad/gl.h>

#include "Texture.hpp"
#include "Util.hpp"
#include "spdlog/spdlog.h"

// For smart pointers
#include <string>
ShadowMapFBO::ShadowMapFBO() : m_fbo(0), m_shadowMap(0), m_rbo(0) {}

ShadowMapFBO::~ShadowMapFBO() {
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }
    if (m_rbo != 0) {
        glDeleteFramebuffers(1, &m_rbo);
    }

    if (m_shadowMap != 0) {
        glDeleteFramebuffers(1, &m_shadowMap);
    }
}

bool ShadowMapFBO::Init(unsigned int WindowWidth, unsigned int WindowHeight) {
    // Create depth buffer
    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, WindowWidth,
                 WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA8,WindowWidth, WindowHeight,
    // 0,GL_RGBA8, GL_UNSIGNED_BYTE, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Create FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, m_shadowMap, 0);

    // Disable color buffer writing
    // glDrawBuffer(GL_NONE);
    glGenTextures(ARRAY_SIZE_IN_ELEMENTS(m_textures), m_textures);
    for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_textures); i++) {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WindowWidth, WindowHeight, 0,
                     GL_RGB, GL_FLOAT, nullptr);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D, m_textures[i], 0);
    }

    GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                            GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

    glDrawBuffers(ARRAY_SIZE_IN_ELEMENTS(DrawBuffers), DrawBuffers);

    // check FBO status
    GLenum const status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
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

GLuint ShadowMapFBO::GetTexture(unsigned num) { return m_textures[num]; }

std::string ShadowMapFBO::CheckShadowTexture() const {
    // Create a temporary object on the stack – no dynamic allocation needed
    Texture2D tempTexture(m_shadowMap, false);
    return tempTexture.GetParameters();
}

void ShadowMapFBO::BindForReading(GLenum TextureUnit) const {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}
