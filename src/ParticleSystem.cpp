#include "ParticleSystem.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>

ParticleSystem::ParticleSystem() {}

ParticleSystem::~ParticleSystem() {}

bool ParticleSystem::Init(glm::vec3 Pos) {
    Particle Particles[MAX_PARTICLES] = {};

    Particles[0].Type = PARTICLE_TYPE_LAUNCHER;
    Particles[0].Pos = Pos;
    Particles[0].Vel = glm::vec3(0.0f, 0.0001f, 0.0f);
    Particles[0].LifetimeMillis = 0.0f;

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

    std::ifstream vertex_shader_file("shaders/particle.vs");
    std::string vertex_shader_text(
        (std::istreambuf_iterator<char>(vertex_shader_file)),
        (std::istreambuf_iterator<char>()));

    std::ifstream fragment_shader_file("shaders/particle.fs");
    std::string fragment_shader_text(
        (std::istreambuf_iterator<char>(fragment_shader_file)),
        (std::istreambuf_iterator<char>()));

    std::ifstream geomentry_shader_file("shaders/particle.gs");
    std::string geometry_shader_text(
        (std::istreambuf_iterator<char>(geomentry_shader_file)),
        (std::istreambuf_iterator<char>()));

    GLuint vertexShaderID = MakeVertexShader(vertex_shader_text);
    GLuint fragmentShaderID = MakeFragmentShader(fragment_shader_text);
    GLuint geometryShaderID = MakeGeometryShader(geometry_shader_text);
    shaderProgramID =
        MakeShaderProgram(vertexShaderID, geometryShaderID, fragmentShaderID);

    if (!m_updateAssistant.Init(shaderProgramID)) {
        return false;
    }

    m_updateAssistant.Enable();

    m_updateAssistant.SetRandomTextureUnit(RANDOM_TEXTURE_UNIT_INDEX);
    m_updateAssistant.SetLauncherLifetime(100.0f);
    m_updateAssistant.SetShellLifetime(10000.0f);
    m_updateAssistant.SetSecondaryShellLifetime(2500.0f);

    if (!m_randomTexture.InitRandomTexture(1000)) {
        return false;
    }

    m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);
    m_colorTexture.Load("Textures/fireworks_red.png");
    m_colorTexture.Bind(COLOR_TEXTURE_UNIT);
    return glGetError() == GL_NO_ERROR;
}

void ParticleSystem::Render(int DeltaTimeMillis, Camera* cam) {
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
                          0);  // type
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

void ParticleSystem::RenderParticles(Camera* cam) {
    m_colorTexture.Bind(COLOR_TEXTURE_UNIT);
    m_updateAssistant.Enable();

    glm::mat4 projection = glm::perspectiveFov(
        cam->GetFov(), static_cast<float>(cam->GetWidth()),
        static_cast<float>(cam->GetHeight()), cam->GetZNear(), cam->GetZFar());
    glm::mat4 view = glm::lookAt(cam->GetPos(), cam->GetTarget(), cam->GetUp());
    glm::mat4 vp_matrix = projection * view;

    // матрица проекции камеры
    // glUniformMatrix4fv(camViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    // позиция камеры
    // glUniform3f(camPosID,cam->GetPos().x,cam->GetPos().y,cam->GetPos().z);

    glUseProgram(shaderProgramID);
    m_billboardAssistant.SetCameraPosition(cam->GetPos());
    m_billboardAssistant.SetVP(glm::value_ptr(vp_matrix));
    m_colorTexture.Bind(COLOR_TEXTURE_UNIT);

    glDisable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                          (const GLvoid*)4);  // position

    glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);

    glDisableVertexAttribArray(0);
}
