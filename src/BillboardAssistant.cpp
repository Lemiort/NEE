#include "BillboardAssistant.h"

#include <glm/gtc/type_ptr.hpp>

BillboardAssistant::BillboardAssistant() {
    // ctor
}

BillboardAssistant::~BillboardAssistant() {
    // dtor
}

bool BillboardAssistant::Init() {
    char* vertexShaderSorceCode = ReadFile("shaders/particle.vs");
    char* fragmentShaderSourceCode = ReadFile("shaders/particle.fs");
    char* geometryShaderSourceCode = ReadFile("shaders/particle.gs");
    GLuint vertexShaderID = MakeVertexShader(vertexShaderSorceCode);
    GLuint fragmentShaderID = MakeFragmentShader(fragmentShaderSourceCode);
    GLuint geometryShaderID = MakeGeometryShader(geometryShaderSourceCode);
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

void BillboardAssistant::SetVP(const glm::mat4& VP) {
    glUniformMatrix4fv(m_VPLocation, 1, GL_TRUE, glm::value_ptr(VP));
}

void BillboardAssistant::SetVP(const GLfloat* VP) {
    glUniformMatrix4fv(m_VPLocation, 1, GL_TRUE, VP);
}

void BillboardAssistant::SetCameraPosition(const glm::vec3& Pos) {
    glUniform3f(m_cameraPosLocation, Pos.x, Pos.y, Pos.z);
}

void BillboardAssistant::SetColorTextureUnit(unsigned int TextureUnit) {
    glUniform1i(m_colorMapLocation, TextureUnit);
}

void BillboardAssistant::SetBillboardSize(float BillboardSize) {
    glUniform1f(m_billboardSizeLocation, BillboardSize);
}
