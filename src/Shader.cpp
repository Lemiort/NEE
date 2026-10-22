#include "Shader.hpp"

#include <glad/gl.h>

#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <array>
#include <fstream>
#include <ios>
#include <limits>
#include <stdexcept>
#include <string>

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
    const auto fileSize = std::filesystem::file_size(filepath);

    // Allocate the string buffer ahead of time
    std::string content;
    if (fileSize > static_cast<std::uintmax_t>(content.max_size()) ||
        fileSize >
            static_cast<std::uintmax_t>(
                std::numeric_limits<std::streamsize>::max())) {
        throw std::length_error("File is too large to read: " +
                                filepath.string());
    }
    content.resize(static_cast<std::size_t>(fileSize));

    // Read the file directly into the string's underlying memory buffer
    file.read(content.data(), static_cast<std::streamsize>(fileSize));
    if (!file) {
        throw std::runtime_error("Failed to read file: " + filepath.string());
    }

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
        throw std::runtime_error("Error creating vertex shader");
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
  FILE const* flog = nullptr;
  // fopen_s(&flog,"shaderbuild.log","w");
  shaderProgramID = glCreateProgram();
  if (shaderProgramID == 0) {
    spdlog::error("Error creating shader program");
    throw std::runtime_error("Error creating shader program");
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
    std::array<GLchar, 1024> errorLog{};
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID,
                            static_cast<GLsizei>(errorLog.size()), nullptr,
                            errorLog.data());
        spdlog::error("Error linking shader program: '{}'", errorLog.data());
        throw std::runtime_error("Error linking shader program");
    }

    glValidateProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID,
                            static_cast<GLsizei>(errorLog.size()), nullptr,
                            errorLog.data());
        spdlog::error("Invalid shader program: '{}'", errorLog.data());
        throw std::runtime_error("Invalid shader program");
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

GLint Shader::GetAttribLocation(const char* attribute) const {
    return glGetAttribLocation(shaderProgramID, attribute);
}

GLint Shader::GetUniformLocation(const char* attribute) const {
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
