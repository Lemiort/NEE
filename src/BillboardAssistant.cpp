#include "BillboardAssistant.hpp"

#include <glad/gl.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "EngineCommon.hpp"
#include "Shader.hpp"
#include "Util.hpp"

BillboardAssistant::BillboardAssistant() = default;

BillboardAssistant::~BillboardAssistant() = default;

bool BillboardAssistant::Init() {
    m_shader = std::make_unique<Shader>();
    m_shader->AddShader(ReadFile("shaders/particle.vsh"),
                        ShaderType::VertexShader);
    m_shader->AddShader(ReadFile("shaders/particle.fsh"),
                        ShaderType::FragmnetShader);
    m_shader->AddShader(ReadFile("shaders/particle.gsh"),
                        ShaderType::GeometryShader);
    m_shader->Init();

    m_VPLocation = m_shader->GetUniformLocation("gVP");
    m_cameraPosLocation = m_shader->GetUniformLocation("gCameraPos");
    m_colorMapLocation = m_shader->GetUniformLocation("gColorMap");
    m_billboardSizeLocation = m_shader->GetUniformLocation("gBillboardSize");

    if (m_VPLocation == INVALID_UNIFORM_LOCATION ||
        m_cameraPosLocation == INVALID_UNIFORM_LOCATION ||
        m_billboardSizeLocation == INVALID_UNIFORM_LOCATION ||
        m_colorMapLocation == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return GLCheckError();
}

void BillboardAssistant::SetVP(const glm::mat4& VP) const {
    glUniformMatrix4fv(m_VPLocation, 1, GL_FALSE, glm::value_ptr(VP));
}

void BillboardAssistant::SetVP(const GLfloat* VP) const {
    glUniformMatrix4fv(m_VPLocation, 1, GL_FALSE, VP);
}

void BillboardAssistant::SetCameraPosition(const glm::vec3& Pos) const {
    glUniform3f(m_cameraPosLocation, Pos.x, Pos.y, Pos.z);
}

void BillboardAssistant::SetColorTextureUnit(GLint textureUnit) const {
    glUniform1i(m_colorMapLocation, textureUnit);
}

void BillboardAssistant::SetBillboardSize(float BillboardSize) const {
    glUniform1f(m_billboardSizeLocation, BillboardSize);
}
