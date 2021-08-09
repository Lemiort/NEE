#ifndef INCLUDE_SHADER_SHADER
#define INCLUDE_SHADER_SHADER

#include <stdexcept>

#include "IShader.h"

namespace nee::shader {

enum class ShaderType : GLenum {
    kVertexShader = GL_VERTEX_SHADER,
    kFragmentShader = GL_FRAGMENT_SHADER,
    kGeometryShader = GL_GEOMETRY_SHADER
};

template <ShaderType shaderType>
class Shader : public IShader {
public:
    Shader(std::string const& source) {
        shaderId = glCreateShader(static_cast<GLenum>(shaderType));
        if (shaderId == 0) {
            throw std::runtime_error("Error on glCreateShader");
        }
        const char* c_str{source.c_str()};
        glShaderSource(shaderId, 1, static_cast<const GLchar**>(&c_str),
                       nullptr);
        glCompileShader(shaderId);
    }
    ~Shader() override = default;
    virtual GLuint GetShaderId() const { return shaderId; }

private:
    GLuint shaderId{0};
};

using VertexShader = Shader<ShaderType::kVertexShader>;
using FragmentShader = Shader<ShaderType::kFragmentShader>;
using GeometryShader = Shader<ShaderType::kGeometryShader>;

}  // namespace nee::shader

#endif /* INCLUDE_SHADER_SHADER */
