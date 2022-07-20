#include "Shader/ShaderProgram.hpp"

#include <array>
#include <stdexcept>

namespace {

inline std::optional<GLint> GLintToOptional(GLint ret) {
    constexpr GLint kInvalidCode = -1;
    if (ret == kInvalidCode) {
        return std::nullopt;
    } else {
        return {ret};
    }
}

constexpr GLint kGlGetProgramivErrorCode = 0;

}  // namespace

namespace nee::shader {

ShaderProgram::ShaderProgram(std::shared_ptr<VertexShader> vertex_shader,
                             std::shared_ptr<FragmentShader> fragment_shader)
    : ShaderProgram(std::move(vertex_shader), std::move(fragment_shader),
                    nullptr) {}

ShaderProgram::ShaderProgram(std::shared_ptr<VertexShader> vertex_shader,
                             std::shared_ptr<FragmentShader> fragment_shader,
                             std::shared_ptr<GeometryShader> geometry_shader)
    : vertex_shader_{std::move(vertex_shader)},
      fragment_shader_{std::move(fragment_shader)},
      geometry_shader_{std::move(geometry_shader)} {
    shader_program_id_ = glCreateProgram();
    if (shader_program_id_ == 0) {
        throw std::runtime_error("Error on glCreateProgram");
    }
    if (vertex_shader_) {
        glAttachShader(shader_program_id_, vertex_shader_->GetShaderId());
    }
    if (fragment_shader_) {
        glAttachShader(shader_program_id_, fragment_shader_->GetShaderId());
    }
    if (geometry_shader_) {
        glAttachShader(shader_program_id_, geometry_shader_->GetShaderId());
    }

    glLinkProgram(shader_program_id_);
    // check
    GLint ret = kGlGetProgramivErrorCode;
    std::array<GLchar, 1024> error_log{};
    glGetProgramiv(shader_program_id_, GL_LINK_STATUS, &ret);
    if (ret == kGlGetProgramivErrorCode) {
        glGetProgramInfoLog(shader_program_id_, std::size(error_log), NULL,
                            error_log.data());
        throw std::runtime_error(
            "Error on glLinkProgram: " +
            std::string(std::begin(error_log), std::end(error_log)));
    }

    glValidateProgram(shader_program_id_);
    glGetProgramiv(shader_program_id_, GL_VALIDATE_STATUS, &ret);
    if (ret == kGlGetProgramivErrorCode) {
        glGetProgramInfoLog(shader_program_id_, std::size(error_log), NULL,
                            error_log.data());
        throw std::runtime_error(
            "Error on glValidateProgram: " +
            std::string(std::begin(error_log), std::end(error_log)));
    }
}
ShaderProgram::~ShaderProgram() {
    if (vertex_shader_) {
        glDetachShader(shader_program_id_, vertex_shader_->GetShaderId());
    }
    if (fragment_shader_) {
        glDetachShader(shader_program_id_, fragment_shader_->GetShaderId());
    }
    if (geometry_shader_) {
        glDetachShader(shader_program_id_, geometry_shader_->GetShaderId());
    }

    glDeleteProgram(shader_program_id_);
}

void ShaderProgram::Use() { glUseProgram(shader_program_id_); }
std::optional<GLint> ShaderProgram::GetAttribLocation(
    const std::string& attribute) {
    auto ret = glGetAttribLocation(shader_program_id_, attribute.c_str());
    return GLintToOptional(ret);
}
std::optional<GLint> ShaderProgram::GetUniformLocation(
    const std::string& uniform) {
    auto ret = glGetUniformLocation(shader_program_id_, uniform.c_str());
    return GLintToOptional(ret);
}

}  // namespace nee::shader
