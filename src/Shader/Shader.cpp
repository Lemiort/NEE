#include "Shader.h"

#include <stdexcept>
#include <vector>

namespace {
GLuint MakeVertexShader(std::string source) {
    GLuint vertex_shader_id_ = glCreateShader(GL_VERTEX_SHADER);
    if (vertex_shader_id_ == 0) {
        throw std::runtime_error("Error creating vertex shader");
    }
    const char* str = source.c_str();
    glShaderSource(vertex_shader_id_, 1, (const GLchar**)&str, NULL);
    glCompileShader(vertex_shader_id_);
    return vertex_shader_id_;
}

GLuint MakeFragmentShader(std::string source) {
    GLuint fragment_shader_id_ = glCreateShader(GL_FRAGMENT_SHADER);
    if (fragment_shader_id_ == 0) {
        throw std::runtime_error("Error creating fragment shader");
    }
    const char* str = source.c_str();
    glShaderSource(fragment_shader_id_, 1, (const GLchar**)&str, NULL);
    glCompileShader(fragment_shader_id_);
    return fragment_shader_id_;
}

GLuint MakeGeometryShader(std::string source) {
    GLuint geometry_shader_id_ = glCreateShader(GL_GEOMETRY_SHADER);
    if (geometry_shader_id_ == 0) {
        throw std::runtime_error("Error creating geometry shader");
    }
    const char* str = source.c_str();
    glShaderSource(geometry_shader_id_, 1, (const GLchar**)&str, NULL);
    glCompileShader(geometry_shader_id_);
    return geometry_shader_id_;
}

GLuint MakeShaderProgram(std::vector<GLuint> shaders) {
    GLuint shaderID = glCreateProgram();
    if (shaderID == 0) {
        throw std::runtime_error("Error creating shader program");
    }
    for (const auto& shader : shaders) {
        glAttachShader(shaderID, shader);
    }
    glLinkProgram(shaderID);

    GLint success = 0;
    GLchar error_log[1024] = {0};
    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(shaderID, sizeof(error_log), NULL, error_log);
        throw std::runtime_error(error_log);
    }

    glValidateProgram(shaderID);
    glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderID, sizeof(error_log), NULL, error_log);
        throw std::runtime_error(error_log);
    }
    return shaderID;
}
}  // namespace

Shader::Shader() {}

Shader::~Shader() {}

void Shader::Use() {
    if (!initialized_) {
        Init();
    }
    glUseProgram(shader_program_id_);
}

void Shader::Add(Type type, std::string source) {
    switch (type) {
        case Shader::Type::kVertex:
            vertex_shader_id_ = MakeVertexShader(source);
            break;
        case Shader::Type::kFragmnet:
            fragment_shader_id_ = MakeFragmentShader(source);
            break;
        case Shader::Type::kGeometry:
            geometry_shader_id_ = MakeGeometryShader(source);
            break;
    }
    initialized_ = false;
}

void Shader::Init() {
    shader_program_id_ = glCreateProgram();
    if (shader_program_id_ == 0) {
        throw std::runtime_error("Error creating shader program");
    }
    if (vertex_shader_id_ != 0)
        glAttachShader(shader_program_id_, vertex_shader_id_);
    if (fragment_shader_id_ != 0)
        glAttachShader(shader_program_id_, fragment_shader_id_);
    if (geometry_shader_id_ != 0)
        glAttachShader(shader_program_id_, geometry_shader_id_);
    glLinkProgram(shader_program_id_);
    //проверка

    GLint success = 0;
    GLchar error_log[1024] = {0};
    glGetProgramiv(shader_program_id_, GL_LINK_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(shader_program_id_, sizeof(error_log), NULL,
                            error_log);
        throw std::runtime_error(error_log);
    }

    glValidateProgram(shader_program_id_);
    glGetProgramiv(shader_program_id_, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program_id_, sizeof(error_log), NULL,
                            error_log);
        throw std::runtime_error(error_log);
    }
    initialized_ = true;
}

GLuint Shader::GetAttribLocation(const char* attribute) {
    return glGetAttribLocation(shader_program_id_, attribute);
}
GLuint Shader::GetUniformLocation(const char* attribute) {
    return glGetUniformLocation(shader_program_id_, attribute);
}

GLuint Shader::GetShaderProgramId() { return shader_program_id_; }