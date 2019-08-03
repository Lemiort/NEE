#include "ParticleUpdateAssistant.h"

ParticleUpdateAssistant::ParticleUpdateAssistant() {
    // ctor
}

ParticleUpdateAssistant::~ParticleUpdateAssistant() {
    // dtor
}
bool ParticleUpdateAssistant::Init(GLuint shader) {
    /*        char* vertexShaderSorceCode=ReadFile("Shaders/particle.vsh");
            char* fragmentShaderSourceCode=ReadFile("Shaders/particle.fsh");
            char* geometryShaderSourceCode=ReadFile("Shaders/particle.gsh");
            GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
            GLuint
       fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode); GLuint
       geometryShaderID=MakeGeometryShader(geometryShaderSourceCode);*
            shaderProgramID=MakeShaderProgram(vertexShaderID,geometryShaderID,
       fragmentShaderID); delete[] vertexShaderSorceCode; delete[]
       fragmentShaderSourceCode; delete[] geometryShaderSourceCode;*/
    shaderProgramID = shader;

    const GLchar* Varyings[4];
    Varyings[0] = "Type1";
    Varyings[1] = "Position1";
    Varyings[2] = "Velocity1";
    Varyings[3] = "Age1";

    glTransformFeedbackVaryings(shaderProgramID, 4, Varyings,
                                GL_INTERLEAVED_ATTRIBS);

    /*if (!Finalize()) {
        return false;
    }*/

    m_deltaTimeMillisLocation =
        glGetUniformLocation(shaderProgramID, "gDeltaTimeMillis");
    m_randomTextureLocation =
        glGetUniformLocation(shaderProgramID, "gRandomTexture");
    m_timeLocation = glGetUniformLocation(shaderProgramID, "gTime");
    m_launcherLifetimeLocation =
        glGetUniformLocation(shaderProgramID, "gLauncherLifetime");
    m_shellLifetimeLocation =
        glGetUniformLocation(shaderProgramID, "gShellLifetime");
    m_secondaryShellLifetimeLocation =
        glGetUniformLocation(shaderProgramID, "gSecondaryShellLifetime");

    if (m_deltaTimeMillisLocation == INVALID_UNIFORM_LOCATION ||
        m_timeLocation == INVALID_UNIFORM_LOCATION ||
        m_randomTextureLocation == INVALID_UNIFORM_LOCATION ||
        m_launcherLifetimeLocation == INVALID_UNIFORM_LOCATION ||
        m_shellLifetimeLocation == INVALID_UNIFORM_LOCATION ||
        m_secondaryShellLifetimeLocation == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
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
