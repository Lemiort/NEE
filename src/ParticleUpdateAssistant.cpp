#include "ParticleUpdateAssistant.hpp"

#include <glad/gl.h>

#include "EngineCommon.hpp"

ParticleUpdateAssistant::ParticleUpdateAssistant() {
    // ctor
}

ParticleUpdateAssistant::~ParticleUpdateAssistant() {
    // dtor
}
bool ParticleUpdateAssistant::Init(const Shader& shader) {
    /*        char* vertexShaderSorceCode=ReadFile("shaders/particle.vsh");
            char* fragmentShaderSourceCode=ReadFile("shaders/particle.fsh");
            char* geometryShaderSourceCode=ReadFile("shaders/particle.gsh");
            GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
            GLuint
       fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode); GLuint
       geometryShaderID=MakeGeometryShader(geometryShaderSourceCode);*
            shaderProgramID=MakeShaderProgram(vertexShaderID,geometryShaderID,
       fragmentShaderID); delete[] vertexShaderSorceCode; delete[]
       fragmentShaderSourceCode; delete[] geometryShaderSourceCode;*/

    const GLchar* Varyings[4];
    Varyings[0] = "Type1";
    Varyings[1] = "Position1";
    Varyings[2] = "Velocity1";
    Varyings[3] = "Age1";

    glTransformFeedbackVaryings(shader.GetShaderProgramId(), 4, Varyings,
                                GL_INTERLEAVED_ATTRIBS);

    /*if (!Finalize()) {
        return false;
    }*/

    m_deltaTimeMillisLocation = shader.GetUniformLocation("gDeltaTimeMillis");
    m_randomTextureLocation = shader.GetUniformLocation("gRandomTexture");
    m_timeLocation = shader.GetUniformLocation("gTime");
    m_launcherLifetimeLocation = shader.GetUniformLocation("gLauncherLifetime");

    m_shellLifetimeLocation = shader.GetUniformLocation("gShellLifetime");
    m_secondaryShellLifetimeLocation =
        shader.GetUniformLocation("gSecondaryShellLifetime");

    return !(m_deltaTimeMillisLocation == INVALID_UNIFORM_LOCATION ||
             m_timeLocation == INVALID_UNIFORM_LOCATION ||
             m_randomTextureLocation == INVALID_UNIFORM_LOCATION ||
             m_launcherLifetimeLocation == INVALID_UNIFORM_LOCATION ||
             m_shellLifetimeLocation == INVALID_UNIFORM_LOCATION ||
             m_secondaryShellLifetimeLocation == INVALID_UNIFORM_LOCATION);
}

void ParticleUpdateAssistant::Enable() { glUseProgram(shaderProgramID); }

void ParticleUpdateAssistant::SetDeltaTimeMillis(float DeltaTimeMillis) {
    glUniform1f(m_deltaTimeMillisLocation, DeltaTimeMillis);
}

void ParticleUpdateAssistant::SetTime(int Time) {
    glUniform1f(m_timeLocation, (float)Time);
}

void ParticleUpdateAssistant::SetRandomTextureUnit(unsigned int TextureUnit) {
    glUniform1i(m_randomTextureLocation, TextureUnit);
}

void ParticleUpdateAssistant::SetLauncherLifetime(float Lifetime) {
    glUniform1f(m_launcherLifetimeLocation, Lifetime);
}

void ParticleUpdateAssistant::SetShellLifetime(float Lifetime) {
    glUniform1f(m_shellLifetimeLocation, Lifetime);
}

void ParticleUpdateAssistant::SetSecondaryShellLifetime(float Lifetime) {
    glUniform1f(m_secondaryShellLifetimeLocation, Lifetime);
}
