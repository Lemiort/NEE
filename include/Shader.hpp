#ifndef SHADER_H
#define SHADER_H
#include <glad/gl.h>

#include <cstddef>
#include <filesystem>
#include <string>

enum ShaderType { VertexShader, GeometryShader, FragmnetShader };

std::string ReadFile(const std::filesystem::path& filepath);
class Shader {
public:
    //  enum ShaderType {Vertex, Geometry, Fragmnet};
    Shader();
    virtual ~Shader();
    void Link();
    void AddShader(const std::string& source, ShaderType type);

    void Use();
    template <std::size_t Size>
    [[nodiscard]] GLint GetAttribLocation(const char (&attribute)[Size]) const {
        return glGetAttribLocation(shaderProgramID, attribute);
    }
    [[nodiscard]] GLint GetAttribLocation(const std::string& attribute) const;
    template <std::size_t Size>
    [[nodiscard]] GLint GetUniformLocation(
        const char (&attribute)[Size]) const {
        return glGetUniformLocation(shaderProgramID, attribute);
    }
    [[nodiscard]] GLint GetUniformLocation(const std::string& attribute) const;

    [[nodiscard]] GLuint GetShaderProgramId() const { return shaderProgramID; };

protected:
private:
    [[nodiscard]] bool TryLoadCachedProgram();
    void CompileAndAttachShaders();
    void SaveProgramBinary() const;

    GLuint shaderProgramID{0};
    GLuint vShader{0};
    GLuint gShader{0};
    GLuint fShader{0};
    std::string vertexSource_;
    std::string geometrySource_;
    std::string fragmentSource_;
    bool initialized{false};
};

#endif  // SHADER_H
