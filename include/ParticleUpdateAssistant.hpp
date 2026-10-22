#ifndef PARTICLEUPDATEASSISTANT_H
#define PARTICLEUPDATEASSISTANT_H
#include <glad/gl.h>

#include "Shader.hpp"

class ParticleUpdateAssistant {
public:
    ParticleUpdateAssistant();
    virtual ~ParticleUpdateAssistant();
    virtual bool Init(const Shader& shader);
    void Enable() const;

    void SetParticleLifetime(float Lifetime);

    void SetDeltaTimeMillis(float DeltaTimeMillis) const;

    void SetTime(int Time) const;

    void SetRandomTextureUnit(GLint textureUnit) const;

    void SetLauncherLifetime(float Lifetime) const;

    void SetShellLifetime(float Lifetime) const;

    void SetSecondaryShellLifetime(float Lifetime) const;

   protected:
   private:
    GLuint shaderProgramID{};

    GLint m_deltaTimeMillisLocation{};
    GLint m_randomTextureLocation{};
    GLint m_timeLocation{};
    GLint m_launcherLifetimeLocation{};
    GLint m_shellLifetimeLocation{};
    GLint m_secondaryShellLifetimeLocation{};
};

#endif  // PARTICLEUPDATEASSISTANT_H
