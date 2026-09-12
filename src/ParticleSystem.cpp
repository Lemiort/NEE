#include "ParticleSystem.hpp"

#include <glad/gl.h>

#include "Assistant.hpp"
#include "Camera.hpp"
#include "EngineCommon.hpp"
#include "Math3d.hpp"
#include "ShaderFunctions.hpp"
#include "Util.hpp"
ParticleSystem::ParticleSystem() = default;

ParticleSystem::~ParticleSystem() = default;

bool ParticleSystem::Init(Vector3f Pos) {
    Particle Particles[MAX_PARTICLES];
    ZERO_MEM(Particles);

    Particles[0].Type = PARTICLE_TYPE_LAUNCHER;
    Particles[0].Pos = Pos;
    Particles[0].Vel = Vector3f(0.0F, 0.0001F, 0.0F);
    Particles[0].LifetimeMillis = 0.0F;

    glGenTransformFeedbacks(2, m_transformFeedback);
    glGenBuffers(2, m_particleBuffer);

    for (unsigned int i = 0; i < 2; i++) {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles,
                     GL_DYNAMIC_DRAW);
    }

    char const* vertexShaderSorceCode = ReadFile("shaders/particle.vsh");
    char const* fragmentShaderSourceCode = ReadFile("shaders/particle.fsh");
    char const* geometryShaderSourceCode = ReadFile("shaders/particle.gsh");
    GLuint const vertexShaderID = MakeVertexShader(vertexShaderSorceCode);
    GLuint const fragmentShaderID =
        MakeFragmentShader(fragmentShaderSourceCode);
    GLuint const geometryShaderID =
        MakeGeometryShader(geometryShaderSourceCode);
    shaderProgramID =
        MakeShaderProgram(vertexShaderID, geometryShaderID, fragmentShaderID);
    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;
    delete[] geometryShaderSourceCode;

    if (!m_updateAssistant.Init(shaderProgramID)) {
        return false;
    }

    m_updateAssistant.Enable();

    m_updateAssistant.SetRandomTextureUnit(RANDOM_TEXTURE_UNIT_INDEX);
    m_updateAssistant.SetLauncherLifetime(100.0F);
    m_updateAssistant.SetShellLifetime(10000.0F);
    m_updateAssistant.SetSecondaryShellLifetime(2500.0F);

    if (!m_randomTexture.InitRandomTexture(1000)) {
        return false;
    }

    m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);
    m_colorTexture.Load("Textures/fireworks_red.png");
    m_colorTexture.Bind(COLOR_TEXTURE_UNIT);
    return GLCheckError();
}

void ParticleSystem::Render(int DeltaTimeMillis, const Camera& cam) {
    m_time += DeltaTimeMillis;

    UpdateParticles(DeltaTimeMillis);

    RenderParticles(cam);

    m_currVB = m_currTFB;
    m_currTFB = (m_currTFB + 1) & 0x1;
}

void ParticleSystem::UpdateParticles(int DeltaTimeMillis) {
    m_updateAssistant.Enable();
    m_updateAssistant.SetTime(m_time);
    m_updateAssistant.SetDeltaTimeMillis(DeltaTimeMillis);

    m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);

    glEnable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,
                            m_transformFeedback[m_currTFB]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          nullptr);  // type
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (const GLvoid*)4);  // position
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (const GLvoid*)16);  // velocity
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (const GLvoid*)28);  // lifetime

    glBeginTransformFeedback(GL_POINTS);

    if (m_isFirst) {
        glDrawArrays(GL_POINTS, 0, 1);

        m_isFirst = false;
    } else {
        glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
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
    Assistant
        TM;  // TM - For object, 2 - for normal of the object, 3 - for position

    // camera for specular
    TM.SetCamera(cam.GetPos(), cam.GetTarget(), cam.GetUp());
    TM.SetPerspectiveProj(cam.GetFov(), cam.GetWidth(), cam.GetHeight(),
                          cam.GetZNear(), cam.GetZFar());
    // Projection matrix of the camera
    //  glUniformMatrix4fv(camViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    // Camera position
    //  glUniform3f(camPosID,cam.GetPos().x,cam.GetPos().y,cam.GetPos().z);

    glUseProgram(shaderProgramID);
    m_billboardAssistant.SetCameraPosition(cam.GetPos());
    m_billboardAssistant.SetVP(reinterpret_cast<const GLfloat*>(TM.GetVC()));
    m_colorTexture.Bind(COLOR_TEXTURE_UNIT);

    glDisable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (const GLvoid*)4);  // position

    glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);

    glDisableVertexAttribArray(0);
}
