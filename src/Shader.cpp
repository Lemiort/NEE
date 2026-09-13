#include "Shader.hpp"

#include <glad/gl.h>

#include <fstream>

#include "spdlog/spdlog.h"

std::string ReadFile(const std::filesystem::path& filepath) {
    // Open in binary mode to ensure accurate byte count and prevent CRLF
    // translation issues
    std::ifstream file(filepath, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        spdlog::error("Failed to open file: {}", filepath.string());
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }

    // Use std::filesystem to get the exact file size
    auto file_size = std::filesystem::file_size(filepath);

    // Allocate the string buffer ahead of time
    std::string content;
    content.resize(file_size);

    // Read the file directly into the string's underlying memory buffer
    file.read(&content[0], file_size);

    return content;
}

GLuint MakeShader(const std::string& source, ShaderType shaderType) {
    GLuint shaderId{};
    switch (shaderType) {
        case ShaderType::FragmnetShader:
            shaderId = glCreateShader(GL_FRAGMENT_SHADER);
            break;
        case ShaderType::GeometryShader:
            shaderId = glCreateShader(GL_GEOMETRY_SHADER);
            break;
        case ShaderType::VertexShader:
            shaderId = glCreateShader(GL_VERTEX_SHADER);
            break;
    }
    if (shaderId == 0) {
        spdlog::error("Error creating vertex shader");
        exit(1);
    }
    const char* c_str = source.c_str();
    glShaderSource(shaderId, 1, &c_str, nullptr);
    glCompileShader(shaderId);
    return shaderId;
}

Shader::Shader() = default;

void Shader::AddShader(const std::string& source, ShaderType type) {
    switch (type) {
        case VertexShader:
            vShader = MakeShader(source, ShaderType::VertexShader);
            break;
        case FragmnetShader:
            fShader = MakeShader(source, ShaderType::FragmnetShader);
            break;
        case GeometryShader:
            gShader = MakeShader(source, ShaderType::GeometryShader);
            break;
    }
}
void Shader::Init() {
    FILE* flog = nullptr;
    // fopen_s(&flog,"shaderbuild.log","w");
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        spdlog::error("Error creating shader program");
        exit(1);
    }
    if (vShader != 0) {
        glAttachShader(shaderProgramID, vShader);
    }
    if (fShader != 0) {
        glAttachShader(shaderProgramID, fShader);
    }
    if (gShader != 0) {
        glAttachShader(shaderProgramID, gShader);
    }
    glLinkProgram(shaderProgramID);
    // check

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), nullptr,
                            ErrorLog);
        spdlog::error("Error linking shader program: '{}'", ErrorLog);
        exit(1);
    }

    glValidateProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), nullptr,
                            ErrorLog);
        spdlog::error("Invalid shader program: '{}'", ErrorLog);
        exit(1);
    }
    // return shaderID;
    initialized = true;
}

void Shader::Use() {
    if (!initialized) {
        Init();
    }
    glUseProgram(shaderProgramID);
}

GLuint Shader::GetAttribLocation(const char* attribute) const {
    return glGetAttribLocation(shaderProgramID, attribute);
}

GLuint Shader::GetUniformLocation(const char* attribute) const {
    return glGetUniformLocation(shaderProgramID, attribute);
}

Shader::~Shader() {
    // dtor
    if (vShader != 0) {
        glDeleteShader(vShader);
    }
    if (fShader != 0) {
        glDeleteShader(fShader);
    }
    if (gShader != 0) {
        glDeleteShader(gShader);
    }
    if (shaderProgramID != 0) {
        glDeleteProgram(shaderProgramID);
    }
}
