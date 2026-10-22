#ifndef SHADER_H
#define SHADER_H
#include <glad/gl.h>

#include <filesystem>
#include <string>

enum ShaderType { VertexShader, GeometryShader, FragmnetShader };

std::string ReadFile(const std::filesystem::path& filepath);
GLuint MakeShader(const std::string& source, ShaderType shaderType);
class Shader {
public:
    //  enum ShaderType {Vertex, Geometry, Fragmnet};
    Shader();
    virtual ~Shader();
    void Init();  // compiles and links the shader
    void AddShader(const std::string& source, ShaderType type);

    void Use();
    [[nodiscard]] GLint GetAttribLocation(const char* attribute) const;
    [[nodiscard]] GLint GetUniformLocation(const char* attribute) const;

    [[nodiscard]] GLuint GetShaderProgramId() const { return shaderProgramID; };

   protected:
   private:
    GLuint shaderProgramID{0};
    GLuint vShader{0};
    GLuint gShader{0};
    GLuint fShader{0};
    bool initialized{false};
};

#endif  // SHADER_H
