#ifndef SHADER_H
#define SHADER_H
#include <glad/gl.h>

#include <string>

using namespace std;
enum ShaderType { VertexShader, GeometryShader, FragmnetShader };
class Shader {
public:
    //  enum ShaderType {Vertex, Geometry, Fragmnet};
    Shader();
    virtual ~Shader();
    void Init();  // compiles and links the shader
    void AddShader(const char* source, ShaderType type);
    GLuint shaderProgramID{0};
    void Use();
    GLuint GetAttribLocation(const char* Attribute) const;
    GLuint GetUniformLocation(const char* Attribute) const;

   protected:
   private:
    string vShaderFileName;
    string fShaderFileName;
    string gShaderFileName;
    GLuint vShader{0};
    GLuint gShader{0};
    GLuint fShader{0};
    bool initialized{false};
};

#endif  // SHADER_H
