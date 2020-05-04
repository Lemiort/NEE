#ifndef BILLBOARDASSISTANT_H
#define BILLBOARDASSISTANT_H
#include <GL/glew.h>

#include "ShaderFunctions.h"
#include "math_3d.h"
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

#endif  // BILLBOARDASSISTANT_H
