#ifndef PARTICLESYSTEM_H_INCLUDED
#define PARTICLESYSTEM_H_INCLUDED
#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
// #include "Billboard.hpp"
#include <memory>

#include "BillboardAssistant.hpp"
#include "Camera.hpp"
#include "ParticleUpdateAssistant.hpp"
#include "RandomTexture.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

inline constexpr std::size_t MAX_PARTICLES = 1000;
inline constexpr float PARTICLE_LIFETIME = 10.0F;
inline constexpr float PARTICLE_TYPE_LAUNCHER = 0.0F;
inline constexpr float PARTICLE_TYPE_SHELL = 1.0F;
inline constexpr float PARTICLE_TYPE_SECONDARY_SHELL = 2.0F;

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();
    bool Init(glm::vec3 Pos);
    void Render(int DeltaTimeMillis, const Camera& cam);

private:
 bool m_isFirst{};
 unsigned int m_currVB{};
 unsigned int m_currTFB{};
 std::array<GLuint, 2> m_particleBuffer{};
 std::array<GLuint, 2> m_transformFeedback{};
 RandomTexture m_randomTexture;
 glm::vec3 m_Pos;
 Texture2D m_colorTexture;
 ParticleUpdateAssistant m_updateAssistant;
 BillboardAssistant m_billboardAssistant;
 int m_time{};
 void RenderParticles(const Camera& cam);
 void UpdateParticles(int DeltaTimeMillis);
 struct Particle {
   float Type{};
   glm::vec3 Pos;
   glm::vec3 Vel;
   float LifetimeMillis{};
 };
    std::unique_ptr<Shader> m_shader;
};

#endif  // PARTICLESYSTEM_H_INCLUDED
