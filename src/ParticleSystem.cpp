#include "ParticleSystem.hpp"

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <stdexcept>

#include "Camera.hpp"
#include "EngineCommon.hpp"
#include "Shader.hpp"
#include "Util.hpp"
#include "spdlog/spdlog.h"

namespace {
constexpr float kInitialParticleVelocity = 0.0001F;
constexpr float kLauncherLifetimeMillis = 100.0F;
constexpr float kShellLifetimeMillis = 10000.0F;
constexpr float kSecondaryShellLifetimeMillis = 2500.0F;
constexpr GLsizei kRandomTextureSize = 1000;

const GLvoid* BufferOffset(std::size_t byteOffset) {
    // OpenGL treats these pointer values as byte offsets when a VBO is bound.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,
    // performance-no-int-to-ptr)
    return reinterpret_cast<const GLvoid*>(
        static_cast<std::uintptr_t>(byteOffset));
}
}  // namespace

ParticleSystem::ParticleSystem(glm::vec3 Pos) {
    try {
        std::array<Particle, MAX_PARTICLES> particles{};

        particles.at(0).Type = PARTICLE_TYPE_LAUNCHER;
        particles.at(0).Pos = Pos;
        particles.at(0).Vel = glm::vec3(0.0F, kInitialParticleVelocity, 0.0F);
        particles.at(0).LifetimeMillis = 0.0F;

        glGenTransformFeedbacks(
            static_cast<GLsizei>(m_transformFeedback.size()),
            m_transformFeedback.data());
        glGenBuffers(static_cast<GLsizei>(m_particleBuffer.size()),
                     m_particleBuffer.data());

        for (std::size_t i = 0; i < m_transformFeedback.size(); ++i) {
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,
                                    m_transformFeedback.at(i));
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
                             m_particleBuffer.at(i));
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
            glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer.at(i));
            glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles.data(),
                         GL_DYNAMIC_DRAW);
        }

        m_shader = std::make_unique<Shader>();
        m_shader->AddShader(ReadFile("shaders/particle.vsh"),
                            ShaderType::VertexShader);
        m_shader->AddShader(ReadFile("shaders/particle.fsh"),
                            ShaderType::FragmnetShader);
        m_shader->AddShader(ReadFile("shaders/particle.gsh"),
                            ShaderType::GeometryShader);
        m_shader->Link();

        if (!m_updateAssistant.Configure(*m_shader)) {
            spdlog::error("Unable to configure particle update shader");
            throw std::runtime_error(
                "Unable to configure particle update shader");
        }

        m_updateAssistant.Enable();

        m_updateAssistant.SetRandomTextureUnit(RANDOM_TEXTURE_UNIT_INDEX);
        m_updateAssistant.SetLauncherLifetime(kLauncherLifetimeMillis);
        m_updateAssistant.SetShellLifetime(kShellLifetimeMillis);
        m_updateAssistant.SetSecondaryShellLifetime(
            kSecondaryShellLifetimeMillis);

        if (!m_randomTexture.InitRandomTexture(kRandomTextureSize)) {
            spdlog::error("Unable to create particle random texture");
            throw std::runtime_error(
                "Unable to create particle random texture");
        }

        m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);
        if (!m_colorTexture.Load("Textures/fireworks_red.png")) {
            spdlog::error("Unable to load particle color texture");
            throw std::runtime_error("Unable to load particle color texture");
        }
        m_colorTexture.Bind(COLOR_TEXTURE_UNIT);
        if (!GLCheckError()) {
            spdlog::error("OpenGL error while constructing particle system");
            throw std::runtime_error(
                "OpenGL error while constructing particle system");
        }
    } catch (...) {
        glDeleteBuffers(static_cast<GLsizei>(m_particleBuffer.size()),
                        m_particleBuffer.data());
        glDeleteTransformFeedbacks(
            static_cast<GLsizei>(m_transformFeedback.size()),
            m_transformFeedback.data());
        throw;
    }
}

ParticleSystem::~ParticleSystem() {
    glDeleteBuffers(static_cast<GLsizei>(m_particleBuffer.size()),
                    m_particleBuffer.data());
    glDeleteTransformFeedbacks(static_cast<GLsizei>(m_transformFeedback.size()),
                               m_transformFeedback.data());
}

void ParticleSystem::Render(int DeltaTimeMillis, const Camera& cam) {
    m_time += DeltaTimeMillis;

    UpdateParticles(DeltaTimeMillis);

    RenderParticles(cam);

    m_currVB = m_currTFB;
    m_currTFB = (m_currTFB + 1U) %
                static_cast<unsigned int>(m_transformFeedback.size());
}

void ParticleSystem::UpdateParticles(int DeltaTimeMillis) {
    m_updateAssistant.Enable();
    m_updateAssistant.SetTime(m_time);
    m_updateAssistant.SetDeltaTimeMillis(static_cast<float>(DeltaTimeMillis));

    m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);

    glEnable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer.at(m_currVB));
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,
                            m_transformFeedback.at(m_currTFB));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          nullptr);  // type
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          BufferOffset(offsetof(Particle, Pos)));  // position
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          BufferOffset(offsetof(Particle, Vel)));  // velocity
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          BufferOffset(offsetof(Particle, LifetimeMillis)));

    glBeginTransformFeedback(GL_POINTS);

    if (m_isFirst) {
        glDrawArrays(GL_POINTS, 0, 1);

        m_isFirst = false;
    } else {
        glDrawTransformFeedback(GL_POINTS, m_transformFeedback.at(m_currVB));
    }

    glEndTransformFeedback();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void ParticleSystem::RenderParticles(const Camera& cam) {
    m_colorTexture.Bind(COLOR_TEXTURE_UNIT);
    m_updateAssistant.Enable();

    m_shader->Use();
    m_billboardAssistant.SetCameraPosition(cam.GetPos());
    m_billboardAssistant.SetVP(cam.GetViewProjectionMatrix());
    m_colorTexture.Bind(COLOR_TEXTURE_UNIT);

    glDisable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer.at(m_currTFB));

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          BufferOffset(offsetof(Particle, Pos)));  // position

    glDrawTransformFeedback(GL_POINTS, m_transformFeedback.at(m_currTFB));

    glDisableVertexAttribArray(0);
}
