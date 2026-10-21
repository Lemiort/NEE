#ifndef PARTICLESYSTEM_H_INCLUDED
#define PARTICLESYSTEM_H_INCLUDED
#include <glad/gl.h>

#include <glm/glm.hpp>
#define MAX_PARTICLES 1000
// #include "Billboard.hpp"
#include <memory>

#include "BillboardAssistant.hpp"
#include "Camera.hpp"
#include "ParticleUpdateAssistant.hpp"
#include "RandomTexture.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#define PARTICLE_LIFETIME 10.0f

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();
    bool Init(glm::vec3 Pos);
    void Render(int DeltaTimeMillis, const Camera& cam);

private:
    bool m_isFirst;
    unsigned int m_currVB;
    unsigned int m_currTFB;
    GLuint m_particleBuffer[2];
    GLuint m_transformFeedback[2];
    RandomTexture m_randomTexture;
    glm::vec3 m_Pos;
    Texture2D m_colorTexture;
    ParticleUpdateAssistant m_updateAssistant;
    BillboardAssistant m_billboardAssistant;
    int m_time;
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
