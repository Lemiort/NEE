#include "BillboardAssistant.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>

BillboardAssistant::BillboardAssistant() {
    // ctor
}

BillboardAssistant::~BillboardAssistant() {
    // dtor
}

bool BillboardAssistant::Init() {
    std::ifstream vertex_shader_file("shaders/particle.vs");
    std::string vertex_shader_text(
        (std::istreambuf_iterator<char>(vertex_shader_file)),
        (std::istreambuf_iterator<char>()));

    std::ifstream fragment_shader_file("shaders/particle.fs");
    std::string fragment_shader_text(
        (std::istreambuf_iterator<char>(fragment_shader_file)),
        (std::istreambuf_iterator<char>()));

    std::ifstream geomentry_shader_file("shaders/particle.gs");
    std::string geometry_shader_text(
        (std::istreambuf_iterator<char>(geomentry_shader_file)),
        (std::istreambuf_iterator<char>()));

    GLuint vertexShaderID = MakeVertexShader(vertex_shader_text);
    GLuint fragmentShaderID = MakeFragmentShader(fragment_shader_text);
    GLuint geometryShaderID = MakeGeometryShader(geometry_shader_text);
    shaderProgramID =
        MakeShaderProgram(vertexShaderID, geometryShaderID, fragmentShaderID);

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

    return glGetError() == GL_NO_ERROR;
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
