#ifndef BILLBOARDASSISTANT_H
#define BILLBOARDASSISTANT_H
#include <GL/glew.h>
#include <util.h>
#include "ShaderFunctions.h"
#include "math_3d.h"
class BillboardAssistant {
public:
    BillboardAssistant();
    virtual ~BillboardAssistant();
    virtual bool Init();
    void SetVP(const Matrix4f& VP);
    void SetVP(const GLfloat* VP);
    void SetCameraPosition(const Vector3f& Pos);
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
