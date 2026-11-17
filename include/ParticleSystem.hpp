#ifndef PARTICLESYSTEM_H_INCLUDED
#define PARTICLESYSTEM_H_INCLUDED
#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <vector>

#include "Camera.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"

// Where particles are born and how they move. Each particle lives for
// `lifetime` seconds, then is born again at a random point of the box with
// `velocity` plus up to `velocityJitter` either way on each axis, and
// speeds up by `acceleration` (gravity, wind) as it goes.
struct ParticleEmitter {
    glm::vec3 boxMin{-1.0F};
    glm::vec3 boxMax{1.0F};
    glm::vec3 velocity{0.0F};
    glm::vec3 velocityJitter{0.0F};
    glm::vec3 acceleration{0.0F};
    float lifetime{1.0F};
};

// How particles are drawn: quads of `width` facing the camera, in `color`
// (which may pass 1 in an HDR target) at `opacity`. With `stretch` above 0
// a particle becomes a streak back along its velocity for that many seconds
// of travel. Additive particles add their light instead of covering.
struct ParticleLook {
    glm::vec3 color{1.0F};
    float opacity{1.0F};
    float width{0.05F};
    float stretch{0.0F};
    bool additive{false};
};

// A fixed number of particles simulated on the GPU with transform feedback,
// ping-ponging between two buffers. Must be created, used and destroyed
// while its OpenGL context is current.
class ParticleSystem {
public:
    struct Particle {
        glm::vec3 position;
        float age;
        glm::vec3 velocity;
        float births;
    };

    // updateShader: shaders/particle_update.vsh alone. renderShader:
    // shaders/particle.vsh, particle.gsh and particle.fsh.
    ParticleSystem(gsl_lite::not_null<std::shared_ptr<Shader>> updateShader,
                   gsl_lite::not_null<std::shared_ptr<Shader>> renderShader,
                   std::size_t count);
    ~ParticleSystem();
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;
    ParticleSystem(ParticleSystem&&) = delete;
    ParticleSystem& operator=(ParticleSystem&&) = delete;

    void SetEmitter(const ParticleEmitter& emitter) { emitter_ = emitter; }
    [[nodiscard]] const ParticleEmitter& GetEmitter() const noexcept {
        return emitter_;
    }
    void SetLook(const ParticleLook& look) { look_ = look; }
    [[nodiscard]] const ParticleLook& GetLook() const noexcept { return look_; }

    // The next update places every particle afresh in the emitter box at a
    // random age, as the first update does.
    void Reset() noexcept { seeded_ = false; }
    // Advances the particles by `deltaSeconds`.
    void Update(float deltaSeconds);
    // Draws the particles depth-tested against the bound framebuffer,
    // without writing depth. Nothing is drawn before the first update.
    // Leaves blending enabled and depth writes off.
    void Render(const Camera& camera) const;

    [[nodiscard]] std::size_t GetCount() const noexcept { return count_; }
    // Copies the particles back from the GPU, for tests and tools.
    [[nodiscard]] std::vector<Particle> ReadParticles() const;

private:
    gsl_lite::not_null<std::shared_ptr<Shader>> updateShader_;
    gsl_lite::not_null<std::shared_ptr<Shader>> renderShader_;
    std::size_t count_;
    std::array<GLuint, 2> buffers_{};
    std::array<VertexArray, 2> vertexArrays_;
    GLuint feedback_{};
    std::size_t current_{0};
    bool seeded_{false};
    ParticleEmitter emitter_;
    ParticleLook look_;
};

#endif  // PARTICLESYSTEM_H_INCLUDED
