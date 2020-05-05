#ifndef INCLUDE_PARTICLEUPDATEASSISTANT_H_
#define INCLUDE_PARTICLEUPDATEASSISTANT_H_
#include <GL/glew.h>

#include "EngineCommon.h"

class ParticleUpdateAssistant {
public:
    ParticleUpdateAssistant();
    virtual ~ParticleUpdateAssistant();
    virtual bool Init(GLuint shader);
    void Enable();

    void SetParticleLifetime(float Lifetime);

    void SetDeltaTimeMillis(float DeltaTimeMillis);

    void SetTime(int Time);

    void SetRandomTextureUnit(unsigned int TextureUnit);

    void SetLauncherLifetime(float Lifetime);

    void SetShellLifetime(float Lifetime);

    void SetSecondaryShellLifetime(float Lifetime);

protected:
private:
    GLuint shaderProgramID;

    GLuint m_deltaTimeMillisLocation;
    GLuint m_randomTextureLocation;
    GLuint m_timeLocation;
    GLuint m_launcherLifetimeLocation;
    GLuint m_shellLifetimeLocation;
    GLuint m_secondaryShellLifetimeLocation;
};

#endif  // INCLUDE_PARTICLEUPDATEASSISTANT_H_
