#include "ParticleUpdateAssistant.hpp"

#include <glad/gl.h>

#include <array>

#include "EngineCommon.hpp"
#include "Shader.hpp"

ParticleUpdateAssistant::ParticleUpdateAssistant() = default;

ParticleUpdateAssistant::~ParticleUpdateAssistant() = default;
bool ParticleUpdateAssistant::Init(const Shader& shader) {
    shaderProgramID = shader.GetShaderProgramId();
    const std::array<const GLchar*, 4> varyings = {
        "Type1", "Position1", "Velocity1", "Age1"};
    glTransformFeedbackVaryings(
        shader.GetShaderProgramId(), static_cast<GLsizei>(varyings.size()),
        varyings.data(), GL_INTERLEAVED_ATTRIBS);

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

    return m_deltaTimeMillisLocation != INVALID_UNIFORM_LOCATION &&
           m_timeLocation != INVALID_UNIFORM_LOCATION &&
           m_randomTextureLocation != INVALID_UNIFORM_LOCATION &&
           m_launcherLifetimeLocation != INVALID_UNIFORM_LOCATION &&
           m_shellLifetimeLocation != INVALID_UNIFORM_LOCATION &&
           m_secondaryShellLifetimeLocation != INVALID_UNIFORM_LOCATION;
}

void ParticleUpdateAssistant::Enable() const { glUseProgram(shaderProgramID); }

void ParticleUpdateAssistant::SetDeltaTimeMillis(float DeltaTimeMillis) const {
  glUniform1f(m_deltaTimeMillisLocation, DeltaTimeMillis);
}

void ParticleUpdateAssistant::SetTime(int Time) const {
  glUniform1f(m_timeLocation, static_cast<float>(Time));
}

void ParticleUpdateAssistant::SetRandomTextureUnit(
    GLint textureUnit) const {
  glUniform1i(m_randomTextureLocation, textureUnit);
}

void ParticleUpdateAssistant::SetLauncherLifetime(float Lifetime) const {
  glUniform1f(m_launcherLifetimeLocation, Lifetime);
}

void ParticleUpdateAssistant::SetShellLifetime(float Lifetime) const {
  glUniform1f(m_shellLifetimeLocation, Lifetime);
}

void ParticleUpdateAssistant::SetSecondaryShellLifetime(float Lifetime) const {
  glUniform1f(m_secondaryShellLifetimeLocation, Lifetime);
}
