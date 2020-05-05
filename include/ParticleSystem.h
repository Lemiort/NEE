#ifndef INCLUDE_PARTICLESYSTEM_H_
#define INCLUDE_PARTICLESYSTEM_H_
#define MAX_PARTICLES 1000
#include "Assistant.h"
#include "BillboardAssistant.h"
#include "Camera.h"
#include "ParticleUpdateAssistant.h"
#include "RandomTexture.h"
#include "ShaderFunctions.h"
#include "Texture.h"
#include "util.h"

#define PARTICLE_LIFETIME 10.0f

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();
    bool Init(glm::vec3 Pos);
    void Render(int DeltaTimeMillis, Camera* cam);

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
    GLuint shaderProgramID;
    BillboardAssistant m_billboardAssistant;
    int m_time;
    void RenderParticles(Camera* cam);
    void UpdateParticles(int DeltaTimeMillis);
    struct Particle {
        float Type;
        glm::vec3 Pos;
        glm::vec3 Vel;
        float LifetimeMillis;
    };
};

#endif  // INCLUDE_PARTICLESYSTEM_H_
