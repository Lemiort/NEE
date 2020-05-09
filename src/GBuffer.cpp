#include "GBuffer.h"

#include <Texture.h>
#include <spdlog/spdlog.h>

bool GBuffer::Init(unsigned int WindowWidth, unsigned int WindowHeight) {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    // create and init color maps, etc
    glGenTextures(std::size(m_textures), m_textures);
    for (unsigned int i = 0; i < std::size(m_textures); i++) {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WindowWidth, WindowHeight, 0,
                     GL_RGB, GL_FLOAT, nullptr);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D, m_textures[i], 0);
    }

    // create depth texture
    glGenTextures(1, &m_depthTexture);
    // init depth texture
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WindowWidth,
    // WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, WindowWidth,
                 WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    /*//TODO check parameters
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);*/
    // glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
    // GL_TEXTURE_2D, m_depthTexture, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                           GL_TEXTURE_2D, m_depthTexture, 0);

    // create and init final color map
    glGenTextures(1, &m_finalTexture);
    glBindTexture(GL_TEXTURE_2D, m_finalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WindowWidth, WindowHeight, 0,
                 GL_RGB, GL_FLOAT, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0 + std::size(m_textures),
                           GL_TEXTURE_2D, m_finalTexture, 0);

    GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                            GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                            GL_COLOR_ATTACHMENT4};

    glDrawBuffers(std::size(DrawBuffers), DrawBuffers);
    spdlog::debug("{} draw buffers", std::size(DrawBuffers));

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            spdlog::debug("Gbuffer complete");
            return true;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            spdlog::error("Gbuffer incomplete: attachment");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            spdlog::error("Gbuffer incomplete: no image is attached to FBO");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            spdlog::error("Gbuffer incomplete: draw buffer");
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            spdlog::error("Gbuffer incomplete: read buffer");
            return false;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            spdlog::error("Gbuffer incomplete: unsupported");
            return false;

        default:
            spdlog::error("Gbuffer incomplete: unknown error");
            return false;
    }

    // return regular fbo
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    return true;
}

void GBuffer::BindForWriting() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void GBuffer::BindForReading() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

    /*for (unsigned int i = 0 ; i < std::size(m_textures); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_POSITION +
    i]);
    }*/
}

void GBuffer::SetReadBuffer(GBUFFER_TEXTURE_TYPE TextureType) {
    glReadBuffer(GL_COLOR_ATTACHMENT0 + TextureType);
}

void GBuffer::CheckTextures() {
    for (unsigned int i = 0; i < std::size(m_textures); i++) {
        Texture2D temp{m_textures[i], false};
        spdlog::debug("Check gbuffer textures: {}", temp.GetParameters());
    }
}

GLuint GBuffer::GetTexture(unsigned num) { return m_textures[num]; }

GLuint GBuffer::GetDepthTexture() { return m_depthTexture; }

void GBuffer::StartFrame() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + std::size(m_textures));
    glClear(GL_COLOR_BUFFER_BIT);
}

void GBuffer::BindForGeomPass() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                            GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                            GL_COLOR_ATTACHMENT4};

    glDrawBuffers(std::size(DrawBuffers), DrawBuffers);
}

void GBuffer::BindForStencilPass() {
    // disable color buffer
    glDrawBuffer(GL_NONE);
}

void GBuffer::BindForLightPass() {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + std::size(m_textures));

    /*for (unsigned int i = 0 ; i < std::size(m_textures); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_POSITION +
    i]);
    }*/
}

void GBuffer::BindForFinalPass() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + std::size(m_textures));
}
