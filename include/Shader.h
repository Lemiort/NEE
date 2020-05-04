#ifndef SHADER_H
#define SHADER_H
#include <GL/glew.h>

#include <string>

using namespace std;
enum ShaderType { VertexShader, GeometryShader, FragmnetShader };
class Shader {
public:
    //  enum ShaderType {Vertex, Geometry, Fragmnet};
    Shader();
    virtual ~Shader();
    void Init();  //собирает и компилирует шейдер
    void AddShader(const char* source, ShaderType type);
    GLuint shaderProgramID;
    void Use();
    GLuint GetAttribLocation(const char* Attribute);
    GLuint GetUniformLocation(const char* Attribute);

protected:
private:
    string vShaderFileName;
    string fShaderFileName;
    string gShaderFileName;
    GLuint vShader;
    GLuint gShader;
    GLuint fShader;
    bool initialized;
};

#endif  // SHADER_H
