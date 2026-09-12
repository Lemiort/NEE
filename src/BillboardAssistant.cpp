#include "BillboardAssistant.hpp"

#include <glad/gl.h>

#include "EngineCommon.hpp"
#include "Math3d.hpp"
#include "ShaderFunctions.hpp"
#include "Util.hpp"

BillboardAssistant::BillboardAssistant() {
    // ctor
}

BillboardAssistant::~BillboardAssistant() {
    // dtor
}

bool BillboardAssistant::Init() {
    char const* vertexShaderSorceCode = ReadFile("shaders/particle.vsh");
    char const* fragmentShaderSourceCode = ReadFile("shaders/particle.fsh");
    char const* geometryShaderSourceCode = ReadFile("shaders/particle.gsh");
    GLuint const vertexShaderID = MakeVertexShader(vertexShaderSorceCode);
    GLuint const fragmentShaderID =
        MakeFragmentShader(fragmentShaderSourceCode);
    GLuint const geometryShaderID =
        MakeGeometryShader(geometryShaderSourceCode);
    shaderProgramID =
        MakeShaderProgram(vertexShaderID, geometryShaderID, fragmentShaderID);
    delete[] vertexShaderSorceCode;
    delete[] fragmentShaderSourceCode;
    delete[] geometryShaderSourceCode;

    m_VPLocation = glGetUniformLocation(shaderProgramID, "gVP");
    // m_VPLocation = GetUniformLocation("gVP");
    m_cameraPosLocation = glGetUniformLocation(shaderProgramID, "gCameraPos");
    m_colorMapLocation = glGetUniformLocation(shaderProgramID, "gColorMap");
    m_billboardSizeLocation =
        glGetUniformLocation(shaderProgramID, "gBillboardSize");

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
