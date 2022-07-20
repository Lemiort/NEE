#ifndef INCLUDE_SHADER_ISHADERPROGRAM
#define INCLUDE_SHADER_ISHADERPROGRAM

#include <GL/glew.h>

#include <optional>
#include <string>

class IShaderProgram {
private:
    /* data */
public:
    IShaderProgram() = default;
    virtual ~IShaderProgram() = default;
    virtual void Use() = 0;
    virtual std::optional<GLint> GetAttribLocation(
        const std::string& attribute) = 0;
    virtual std::optional<GLint> GetUniformLocation(
        const std::string& attribute) = 0;
};

#endif /* INCLUDE_SHADER_ISHADERPROGRAM */
