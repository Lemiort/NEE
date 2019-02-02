#ifndef SHADERSHADER_H
#define SHADERSHADER_H

#include <GL/glew.h>

#include <string>

class Shader {
public:
    enum class Type : int {
        kVertex = GL_VERTEX_SHADER,
        kGeometry = GL_GEOMETRY_SHADER,
        kFragmnet = GL_FRAGMENT_SHADER
    };
    Shader();
    virtual ~Shader();
    void Use();
    void Add(Type type, std::string source);
    GLuint GetAttribLocation(const char* Attribute);
    GLuint GetUniformLocation(const char* Attribute);
    // TODO remove
    GLuint GetShaderProgramId();

private:
    GLuint vertex_shader_id_ = 0;
    GLuint fragment_shader_id_ = 0;
    GLuint geometry_shader_id_ = 0;
    GLuint shader_program_id_ = 0;
    bool initialized_ = false;
    void Init();
};

#endif /* SHADERSHADER_H */
