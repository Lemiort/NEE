#include "BloomPass.hpp"

#include <algorithm>
#include <utility>

BloomPass::BloomPass(
    gsl_lite::not_null<std::shared_ptr<Shader>> downsampleShader,
    gsl_lite::not_null<std::shared_ptr<Shader>> upsampleShader)
    : downsampleShader_(std::move(downsampleShader)),
      upsampleShader_(std::move(upsampleShader)) {
    glCreateFramebuffers(1, &framebuffer_);
}

BloomPass::~BloomPass() {
    Release();
    glDeleteFramebuffers(1, &framebuffer_);
}

void BloomPass::Release() {
    for (const Level& level : levels_) {
        glDeleteTextures(1, &level.texture);
    }
    levels_.clear();
}

void BloomPass::Resize(GLsizei width, GLsizei height) {
    Release();
    width_ = width;
    height_ = height;
    GLsizei levelWidth = width / 2;
    GLsizei levelHeight = height / 2;
    // Halve until the next level would be smaller than 2 texels a side.
    while (levels_.size() < kMaxLevels && levelWidth >= 2 && levelHeight >= 2) {
        Level level{.width = levelWidth, .height = levelHeight};
        glCreateTextures(GL_TEXTURE_2D, 1, &level.texture);
        glTextureStorage2D(level.texture, 1, GL_RGBA16F, levelWidth,
                           levelHeight);
        glTextureParameteri(level.texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(level.texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(level.texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(level.texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        levels_.push_back(level);
        levelWidth /= 2;
        levelHeight /= 2;
    }
}

GLuint BloomPass::Render(GLuint source, GLsizei width, GLsizei height,
                         const BloomSettings& settings) {
    if (width != width_ || height != height_) {
        Resize(width, height);
    }
    if (levels_.empty()) {
        return 0;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    vertexArray_.Bind();
    const auto pass = [this](Shader& shader, GLuint input, GLsizei inputWidth,
                             GLsizei inputHeight, const Level& target) {
        glNamedFramebufferTexture(framebuffer_, GL_COLOR_ATTACHMENT0,
                                  target.texture, 0);
        glNamedFramebufferDrawBuffer(framebuffer_, GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, target.width, target.height);
        glBindTextureUnit(0, input);
        glUniform1i(shader.GetUniformLocation("source"), 0);
        glUniform2f(shader.GetUniformLocation("sourceTexel"),
                    1.0F / static_cast<float>(inputWidth),
                    1.0F / static_cast<float>(inputHeight));
        glUniform2f(shader.GetUniformLocation("targetSize"),
                    static_cast<float>(target.width),
                    static_cast<float>(target.height));
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };

    Shader& down = *downsampleShader_;
    down.Use();
    glUniform1f(down.GetUniformLocation("threshold"), settings.threshold);
    glUniform1f(down.GetUniformLocation("knee"),
                std::max(settings.softKnee, 0.0F));
    GLuint input = source;
    GLsizei inputWidth = width;
    GLsizei inputHeight = height;
    for (std::size_t index = 0; index < levels_.size(); ++index) {
        glUniform1i(down.GetUniformLocation("firstPass"),
                    index == 0 ? GL_TRUE : GL_FALSE);
        const Level& level = levels_[index];
        pass(down, input, inputWidth, inputHeight, level);
        input = level.texture;
        inputWidth = level.width;
        inputHeight = level.height;
    }

    Shader& up = *upsampleShader_;
    up.Use();
    glUniform1f(up.GetUniformLocation("radius"), settings.radius);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    for (std::size_t index = levels_.size() - 1; index > 0; --index) {
        const Level& smaller = levels_[index];
        pass(up, smaller.texture, smaller.width, smaller.height,
             levels_[index - 1]);
    }
    glDisable(GL_BLEND);
    return levels_.front().texture;
}
