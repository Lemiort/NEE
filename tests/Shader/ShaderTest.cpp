#include "Shader/Shader.h"
#include "gtest/gtest.h"

using namespace nee::shader;

namespace {

constexpr char kVertexShaderText[] =

    "#version 330                                                                   \n\
                                                                                    \n\
    layout (location = 0) in vec3 Position;                                         \n\
                                                                                    \n\
    void main()                                                                     \n\
    {                                                                               \n\
        gl_Position = vec4(0.5 * Position.x, 0.5 * Position.y, Position.z, 1.0);    \n\
    }";

constexpr char kFragmentShaderText[] =
    "#version 330                                                                   \n\
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