#ifndef PARTICLESYSTEM_H_INCLUDED
#define PARTICLESYSTEM_H_INCLUDED
#define MAX_PARTICLES 1000
// #include "Billboard.hpp"
#include "Assistant.hpp"
#include "BillboardAssistant.hpp"
#include "Camera.hpp"
#include "ParticleUpdateAssistant.hpp"
#include "RandomTexture.hpp"
#include "ShaderFunctions.hpp"
#include "Texture.hpp"
#include "Util.hpp"

#define PARTICLE_LIFETIME 10.0f

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();
    bool Init(Vector3f Pos);
    void Render(int DeltaTimeMillis, const Camera& cam);

private:
    bool m_isFirst;
    unsigned int m_currVB;
    unsigned int m_currTFB;
    GLuint m_particleBuffer[2];
    GLuint m_transformFeedback[2];
    RandomTexture m_randomTexture;
    Vector3f m_Pos;
    Texture2D m_colorTexture;
    ParticleUpdateAssistant m_updateAssistant;
    GLuint shaderProgramID;
    BillboardAssistant m_billboardAssistant;
    int m_time;
    void RenderParticles(const Camera& cam);
    void UpdateParticles(int DeltaTimeMillis);
    struct Particle {
        float Type;
        Vector3f Pos;
        Vector3f Vel;
        float LifetimeMillis;
    };
};

#endif  // PARTICLESYSTEM_H_INCLUDED
