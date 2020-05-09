#include "ShadowMapFBO.h"

#include <spdlog/spdlog.h>

#include "Texture.h"

ShadowMapFBO::ShadowMapFBO() {
    m_fbo = 0;
    m_shadowMap = 0;
    m_rbo = 0;
}

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
    // create depth buffer
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

    // create fbo
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, m_shadowMap, 0);

    // disable writing to color buffer
    // glDrawBuffer(GL_NONE);
    glGenTextures(std::size(m_textures), m_textures);
    for (unsigned int i = 0; i < std::size(m_textures); i++) {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WindowWidth, WindowHeight, 0,
                     GL_RGB, GL_FLOAT, nullptr);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D, m_textures[i], 0);
    }

    GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                            GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

    glDrawBuffers(std::size(DrawBuffers), DrawBuffers);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            spdlog::debug("Framebuffer complete");
            return true;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            spdlog::error("Framebuffer incomplete: attachment");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            spdlog::error(
                "Framebuffer incomplete: no image is attached to FBO");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            spdlog::error("Framebuffer incomplete: draw buffer");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            spdlog::error("Framebuffer incomplete: read buffer");
            return false;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            spdlog::error("Framebuffer incomplete: unsupported");
            return false;

        default:
            spdlog::error("Framebuffer incomplete: unknown error");
            return false;
    }
}

void ShadowMapFBO::BindForWriting() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

GLuint ShadowMapFBO::GetTexture() { return m_shadowMap; }

GLuint ShadowMapFBO::GetTexture(unsigned num) { return m_textures[num]; }

std::string ShadowMapFBO::CheckShadowTexture() {
    Texture2D tempTexture{m_shadowMap, false};
    return tempTexture.GetParameters();
}

void ShadowMapFBO::BindForReading(GLenum TextureUnit) {
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}
