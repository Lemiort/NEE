#include "ShaderFunctions.h"

#include <spdlog/spdlog.h>

#include <fstream>

GLuint MakeVertexShader(const std::string& source) {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if (vertexShaderID == 0) {
        spdlog::error("Error creating  vertex shader");
        exit(1);
    }
    const char* c_str{source.c_str()};
    glShaderSource(vertexShaderID, 1, static_cast<const GLchar**>(&c_str),
                   nullptr);
    glCompileShader(vertexShaderID);
    return vertexShaderID;
}

GLuint MakeFragmentShader(const std::string& source) {
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragmentShaderID == 0) {
        spdlog::error("Error creating  fragment shader");
        exit(1);
    }
    const char* c_str{source.c_str()};
    glShaderSource(fragmentShaderID, 1, static_cast<const GLchar**>(&c_str),
                   nullptr);
    glCompileShader(fragmentShaderID);
    return fragmentShaderID;
}

GLuint MakeGeometryShader(const std::string& source) {
    GLuint geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
    if (geometryShaderID == 0) {
        spdlog::error("Error creating  fragment shader");
        exit(1);
    }
    const char* c_str{source.c_str()};
    glShaderSource(geometryShaderID, 1, static_cast<const GLchar**>(&c_str),
                   nullptr);
    glCompileShader(geometryShaderID);
    return geometryShaderID;
}

GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
    GLuint shaderID = glCreateProgram();
    if (shaderID == 0) {
        spdlog::error("Error creating  shader program");
        exit(1);
    }
    glAttachShader(shaderID, vertexShaderID);
    glAttachShader(shaderID, fragmentShaderID);
    glLinkProgram(shaderID);

    GLint Success = 0;
    GLchar errorLog[1024] = {0};
    glGetProgramiv(shaderID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(errorLog), NULL, errorLog);
        spdlog::error("Error linking shader program {}", errorLog);
        exit(1);
    }

    glValidateProgram(shaderID);
    glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderID, sizeof(errorLog), NULL, errorLog);
        spdlog::error("Invalid shader program: {}", errorLog);
        exit(1);
    }
    return shaderID;
}
GLuint MakeShaderProgram(GLuint vertexShaderID, GLuint geometryShaderID,
                         GLuint fragmentShaderID) {
    GLuint shaderID = glCreateProgram();
    if (shaderID == 0) {
        spdlog::error("Error creating  shader program");
        exit(1);
    }
    glAttachShader(shaderID, vertexShaderID);
    glAttachShader(shaderID, fragmentShaderID);
    glAttachShader(shaderID, geometryShaderID);
    glLinkProgram(shaderID);

    GLint Success = 0;
    GLchar errorLog[1024] = {0};
    glGetProgramiv(shaderID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(errorLog), NULL, errorLog);
        spdlog::error("Error linking  shader program: ", errorLog);
        exit(1);
    }

    glValidateProgram(shaderID);
    glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderID, sizeof(errorLog), NULL, errorLog);
        spdlog::error("Invalid shader program: ", errorLog);
        exit(1);
    }
    return shaderID;
}

std::string ReadFromFile(const std::string& path) {
    std::ifstream file(path);
    std::string text((std::istreambuf_iterator<char>(file)),
                     (std::istreambuf_iterator<char>()));
    return text;
}
