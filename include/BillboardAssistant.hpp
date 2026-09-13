#ifndef BILLBOARDASSISTANT_H
#define BILLBOARDASSISTANT_H
#include <glad/gl.h>

#include <memory>

#include "Math3d.hpp"
#include "Shader.hpp"
class BillboardAssistant {
public:
    BillboardAssistant();
    virtual ~BillboardAssistant();
    virtual bool Init();
    void SetVP(const Matrix4f& VP) const;
    void SetVP(const GLfloat* VP) const;
    void SetCameraPosition(const Vector3f& Pos) const;
    void SetColorTextureUnit(unsigned int TextureUnit) const;
    void SetBillboardSize(float BillboardSize) const;

private:
    GLuint m_VPLocation{};
    GLuint m_cameraPosLocation{};
    GLuint m_colorMapLocation{};
    GLuint m_billboardSizeLocation{};
    std::unique_ptr<Shader> m_shader;
};

#endif  // BILLBOARDASSISTANT_H
