#include "GBuffer.hpp"

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Texture.hpp"
#include "spdlog/spdlog.h"

namespace {
struct PendingGBuffer {
    GLuint fbo{};
    std::array<GLuint, GBuffer::GBUFFER_NUM_TEXTURES> textures{};
    GLuint depthTexture{};
    GLuint finalTexture{};

    ~PendingGBuffer() {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(static_cast<GLsizei>(textures.size()),
                         textures.data());
        glDeleteTextures(1, &depthTexture);
        glDeleteTextures(1, &finalTexture);
    }
};

bool HasAllResources(const PendingGBuffer& resources) {
    if (resources.fbo == 0 || resources.depthTexture == 0 ||
        resources.finalTexture == 0) {
        return false;
    }
    for (const GLuint texture : resources.textures) {
        if (texture == 0) return false;
    }
    return true;
}
}  // namespace

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
    PendingGBuffer replacement;
    // Create FBO
    glGenFramebuffers(1, &replacement.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, replacement.fbo);

    // create and initialize color maps and others
    glGenTextures(static_cast<GLsizei>(replacement.textures.size()),
                  replacement.textures.data());
    for (std::size_t i = 0; i < replacement.textures.size(); ++i) {
        glBindTexture(GL_TEXTURE_2D, replacement.textures.at(i));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0,
                     GL_RGB, GL_FLOAT, nullptr);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                               GL_TEXTURE_2D, replacement.textures.at(i), 0);
    }

    // Create a combined depth/stencil texture. The external format and type
    // must match GL_DEPTH32F_STENCIL8 even when no initial data is supplied.
    glGenTextures(1, &replacement.depthTexture);
    glBindTexture(GL_TEXTURE_2D, replacement.depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, windowWidth,
                 windowHeight, 0, GL_DEPTH_STENCIL,
                 GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                           GL_TEXTURE_2D, replacement.depthTexture, 0);

    // create and initialize final color buffer
    glGenTextures(1, &replacement.finalTexture);
    glBindTexture(GL_TEXTURE_2D, replacement.finalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0,
                 GL_RGB, GL_FLOAT, nullptr);
    glFramebufferTexture2D(
        GL_DRAW_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(replacement.textures.size()),
        GL_TEXTURE_2D, replacement.finalTexture, 0);

    const std::array<GLenum, 5> drawBuffers = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};

    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    spdlog::info("draw buffers: {}", drawBuffers.size());

    // check FBO status
    const GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (!HasAllResources(replacement)) {
        spdlog::error("Unable to allocate all G-buffer resources");
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        return false;
    }
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            spdlog::info("Framebuffer (GBuffer) complete.");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            spdlog::error(
                "Framebuffer incomplete: Attachment is NOT complete.");
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            spdlog::error(
                "Framebuffer incomplete: No image is attached to FBO.");
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            spdlog::error("Framebuffer incomplete: Draw buffer.");
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            return false;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            spdlog::error("Framebuffer incomplete: Read buffer.");
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            return false;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            spdlog::error(
                "Framebuffer incomplete: Unsupported by FBO implementation.");
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            return false;

        default:
            spdlog::error("Framebuffer incomplete: Unknown error.");
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            return false;
    }

    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(static_cast<GLsizei>(m_textures.size()),
                     m_textures.data());
    glDeleteTextures(1, &m_depthTexture);
    glDeleteTextures(1, &m_testTexture);
    glDeleteTextures(1, &m_finalTexture);
    m_fbo = std::exchange(replacement.fbo, 0);
    m_textures = std::exchange(replacement.textures, {});
    m_depthTexture = std::exchange(replacement.depthTexture, 0);
    m_testTexture = 0;
    m_finalTexture = std::exchange(replacement.finalTexture, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

    return true;
}

void GBuffer::BindForWriting() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void GBuffer::BindForReading() const {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);

    /*for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_textures); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_POSITION +
    i]);
    }*/
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

    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
}

void GBuffer::BindForStencilPass() {
    // should disable color buffer
    glDrawBuffer(GL_NONE);
}

void GBuffer::BindForLightPass() {
    glDrawBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_textures.size()));

    /*for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_textures); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_POSITION +
    i]);
    }*/
}

void GBuffer::BindForFinalPass() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_textures.size()));
}
