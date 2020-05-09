#ifndef INCLUDE_SHADER_H
#define INCLUDE_SHADER_H
#include <GL/glew.h>

#include <string>

enum ShaderType { kVertexShader, kGeometryShader, kFragmentShader };

class Shader {
public:
    //  enum ShaderType {Vertex, Geometry, Fragmnet};
    Shader();
    virtual ~Shader();
    void Init();  // собирает и компилирует шейдер
    void AddShader(const std::string& source, ShaderType type);
    GLuint shaderProgramID;
    void Use();
    GLuint GetAttribLocation(const char* Attribute);
    GLuint GetUniformLocation(const char* Attribute);

protected:
private:
    std::string vShaderFileName;
    std::string fShaderFileName;
    std::string gShaderFileName;
    GLuint vShader;
    GLuint gShader;
    GLuint fShader;
    bool initialized;
};

#endif  // INCLUDE_SHADER_H
