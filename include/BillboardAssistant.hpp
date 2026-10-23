#ifndef BILLBOARDASSISTANT_H
#define BILLBOARDASSISTANT_H
#include <glad/gl.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>

#include "Shader.hpp"
class BillboardAssistant {
public:
    BillboardAssistant();
    virtual ~BillboardAssistant();
    void SetVP(const glm::mat4& VP) const;
    void SetCameraPosition(const glm::vec3& Pos) const;
    void SetColorTextureUnit(GLint textureUnit) const;
    void SetBillboardSize(float BillboardSize) const;

private:
    GLint m_VPLocation{};
    GLint m_cameraPosLocation{};
    GLint m_colorMapLocation{};
    GLint m_billboardSizeLocation{};
    std::unique_ptr<Shader> m_shader;
};

#endif  // BILLBOARDASSISTANT_H
