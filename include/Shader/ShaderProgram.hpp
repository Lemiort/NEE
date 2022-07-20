#ifndef INCLUDE_SHADER_SHADERPROGRAM
#define INCLUDE_SHADER_SHADERPROGRAM
#include <memory>

#include "IShaderProgram.hpp"
#include "Shader.hpp"

namespace nee::shader {
class ShaderProgram : public IShaderProgram {
private:
    /* data */
    GLuint shader_program_id_{0};
    std::shared_ptr<VertexShader> vertex_shader_{};
    std::shared_ptr<FragmentShader> fragment_shader_{};
    std::shared_ptr<GeometryShader> geometry_shader_{};

public:
    ShaderProgram(std::shared_ptr<VertexShader> vertex_shader,
                  std::shared_ptr<FragmentShader> fragment_shader);
    ShaderProgram(std::shared_ptr<VertexShader> vertex_shader,
                  std::shared_ptr<FragmentShader> fragment_shader,
                  std::shared_ptr<GeometryShader> geometry_shader);
    ~ShaderProgram() override;

    void Use() override;
    std::optional<GLint> GetAttribLocation(
        const std::string& attribute) override;
    std::optional<GLint> GetUniformLocation(
        const std::string& uniform) override;
};
}  // namespace nee::shader

#endif /* INCLUDE_SHADER_SHADERPROGRAM */
