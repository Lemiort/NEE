#ifndef BLOOM_PASS_HPP_INCLUDED
#define BLOOM_PASS_HPP_INCLUDED

#include <glad/gl.h>

#include <cstddef>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <vector>

#include "Shader.hpp"
#include "VertexArray.hpp"

// How much light spills around bright surfaces. Light brighter than
// `threshold` (its largest channel), fading in over `softKnee` below it, is
// blurred wide and added to the frame times `strength`. A strength of 0
// turns bloom off and skips its passes.
struct BloomSettings {
    float strength{0.0F};
    float threshold{1.0F};
    float softKnee{0.5F};
    // Spread of each upsampling step, in texels of the smaller level.
    float radius{1.0F};
};

// Blurs the bright light of a frame through a chain of half-float levels,
// each half the size of the one before: downsample with a threshold, then
// upsample back, adding each level onto the larger one.
class BloomPass {
public:
    // shaders/fullscreen.vsh with bloom_downsample.fsh and bloom_upsample.fsh.
    BloomPass(gsl_lite::not_null<std::shared_ptr<Shader>> downsampleShader,
              gsl_lite::not_null<std::shared_ptr<Shader>> upsampleShader);
    ~BloomPass();
    BloomPass(const BloomPass&) = delete;
    BloomPass& operator=(const BloomPass&) = delete;
    BloomPass(BloomPass&&) = delete;
    BloomPass& operator=(BloomPass&&) = delete;

    // Returns the bloom of `source` (width x height), at half that size. The
    // levels follow the source size. Leaves the pass's framebuffer bound.
    GLuint Render(GLuint source, GLsizei width, GLsizei height,
                  const BloomSettings& settings);
    [[nodiscard]] std::size_t GetLevelCount() const noexcept {
        return levels_.size();
    }

    static constexpr std::size_t kMaxLevels = 6;

private:
    struct Level {
        GLuint texture{};
        GLsizei width{};
        GLsizei height{};
    };
    void Resize(GLsizei width, GLsizei height);
    void Release();

    gsl_lite::not_null<std::shared_ptr<Shader>> downsampleShader_;
    gsl_lite::not_null<std::shared_ptr<Shader>> upsampleShader_;
    std::vector<Level> levels_;
    GLuint framebuffer_{};
    GLsizei width_{};
    GLsizei height_{};
    VertexArray vertexArray_;
};

#endif  // BLOOM_PASS_HPP_INCLUDED
