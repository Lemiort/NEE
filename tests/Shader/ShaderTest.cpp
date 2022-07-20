#include "Shader/Shader.hpp"
#include "Shader/ShaderProgram.hpp"
#include "gtest/gtest.h"

using namespace nee::shader;

namespace {

constexpr char kVertexShaderText[] =

    "#version 460                                                                   \n\
                                                                                    \n\
    layout (location = 0) in vec3 Position;                                         \n\
                                                                                    \n\
    uniform float some_uniform;                                                      \n\
                                                                                    \n\
    void main()                                                                     \n\
    {                                                                               \n\
        gl_Position = vec4(0.5 * Position.x, 0.5 * Position.y, Position.z*some_uniform, 1.0);    \n\
    }";

constexpr char kFragmentShaderText[] =
    "#version 460                                                                   \n\
                                                                                    \n\
    out vec4 FragColor;                                                             \n\
                                                                                    \n\
    void main()                                                                     \n\
    {                                                                               \n\
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);                                       \n\
    }";
}  // namespace

TEST(VertexShaderTest, SimpleConstructor) {
    // Arrange
    std::unique_ptr<VertexShader> shader;

    // Act
    shader = std::make_unique<VertexShader>(kVertexShaderText);

    // Assert
    EXPECT_NE(shader, nullptr);
}

TEST(FragmentShaderTest, SimpleConstructor) {
    // Arrange
    std::unique_ptr<FragmentShader> shader;

    // Act
    shader = std::make_unique<FragmentShader>(kFragmentShaderText);

    // Assert
    EXPECT_NE(shader, nullptr);
}

TEST(ShaderProgramTest, SimpleConstructor) {
    // Arrange
    auto vertex_shader = std::make_shared<VertexShader>(kVertexShaderText);
    auto fragment_shader =
        std::make_shared<FragmentShader>(kFragmentShaderText);
    std::unique_ptr<ShaderProgram> shader_program;

    // Act
    shader_program = std::make_unique<ShaderProgram>(
        std::move(vertex_shader), std::move(fragment_shader));

    // Assert
    EXPECT_NE(shader_program, nullptr);
}
TEST(ShaderProgramTest, UseTest) {
    // Arrange
    auto vertex_shader = std::make_shared<VertexShader>(kVertexShaderText);
    auto fragment_shader =
        std::make_shared<FragmentShader>(kFragmentShaderText);
    auto shader_program = std::make_unique<ShaderProgram>(
        std::move(vertex_shader), std::move(fragment_shader));

    // Act & Assert
    EXPECT_NO_THROW(shader_program->Use());
}

TEST(ShaderProgramTest, AttributeTest) {
    // Arrange
    auto vertex_shader = std::make_shared<VertexShader>(kVertexShaderText);
    auto fragment_shader =
        std::make_shared<FragmentShader>(kFragmentShaderText);
    auto shader_program = std::make_unique<ShaderProgram>(
        std::move(vertex_shader), std::move(fragment_shader));
    EXPECT_NO_THROW(shader_program->Use());

    // Act
    auto attribute = shader_program->GetAttribLocation("Position");
    auto no_attribute = shader_program->GetAttribLocation("dummy");

    // Assert
    EXPECT_TRUE(attribute);
    EXPECT_FALSE(no_attribute);
}

TEST(ShaderProgramTest, UniformsTest) {
    // Arrange
    auto vertex_shader = std::make_shared<VertexShader>(kVertexShaderText);
    auto fragment_shader =
        std::make_shared<FragmentShader>(kFragmentShaderText);
    auto shader_program = std::make_unique<ShaderProgram>(
        std::move(vertex_shader), std::move(fragment_shader));
    EXPECT_NO_THROW(shader_program->Use());

    // Act
    auto uniform = shader_program->GetUniformLocation("some_uniform");
    auto no_uniform = shader_program->GetUniformLocation("dummy");

    // Assert
    EXPECT_TRUE(uniform) << *uniform;
    EXPECT_FALSE(no_uniform);
}