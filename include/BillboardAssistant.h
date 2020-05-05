#ifndef INCLUDE_BILLBOARDASSISTANT_H_
#define INCLUDE_BILLBOARDASSISTANT_H_
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "ShaderFunctions.h"
#include "util.h"

class BillboardAssistant {
public:
    BillboardAssistant();
    virtual ~BillboardAssistant();
    virtual bool Init();
    void SetVP(const glm::mat4& VP);
    void SetVP(const GLfloat* VP);
    void SetCameraPosition(const glm::vec3& Pos);
    void SetColorTextureUnit(unsigned int TextureUnit);
    void SetBillboardSize(float BillboardSize);

private:
    GLuint m_VPLocation;
    GLuint m_cameraPosLocation;
    GLuint m_colorMapLocation;
    GLuint m_billboardSizeLocation;
    GLuint shaderProgramID;
};

#endif  // INCLUDE_BILLBOARDASSISTANT_H_
