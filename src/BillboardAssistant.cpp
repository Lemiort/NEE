#include "BillboardAssistant.hpp"

#include <glad/gl.h>

#include "EngineCommon.hpp"
#include "Math3d.hpp"
#include "Util.hpp"

BillboardAssistant::BillboardAssistant() {
    // ctor
}

BillboardAssistant::~BillboardAssistant() {
    // dtor
}

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

void BillboardAssistant::SetVP(const Matrix4f& VP) const {
    glUniformMatrix4fv(m_VPLocation, 1, GL_TRUE,
                       reinterpret_cast<const GLfloat*>(VP.m));
}

void BillboardAssistant::SetVP(const GLfloat* VP) const {
    glUniformMatrix4fv(m_VPLocation, 1, GL_TRUE, VP);
}

void BillboardAssistant::SetCameraPosition(const Vector3f& Pos) const {
    glUniform3f(m_cameraPosLocation, Pos.x, Pos.y, Pos.z);
}

void BillboardAssistant::SetColorTextureUnit(unsigned int TextureUnit) const {
    glUniform1i(m_colorMapLocation, TextureUnit);
}

void BillboardAssistant::SetBillboardSize(float BillboardSize) const {
    glUniform1f(m_billboardSizeLocation, BillboardSize);
}
