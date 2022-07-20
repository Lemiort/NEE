#ifndef INCLUDE_SHADER_SHADER
#define INCLUDE_SHADER_SHADER

#include <stdexcept>

#include "IShader.hpp"

namespace nee::shader {

enum class ShaderType : GLenum {
    kVertexShader = GL_VERTEX_SHADER,
    kFragmentShader = GL_FRAGMENT_SHADER,
    kGeometryShader = GL_GEOMETRY_SHADER
};

template <ShaderType shader_type>
class Shader : public IShader {
public:
    explicit Shader(std::string const& source) {
        shader_id_ = glCreateShader(static_cast<GLenum>(shader_type));
        if (shader_id_ == 0) {
            throw std::runtime_error("Error on glCreateShader");
        }
        const char* c_str{source.c_str()};
        glShaderSource(shader_id_, 1, static_cast<const GLchar**>(&c_str),
                       nullptr);
        glCompileShader(shader_id_);
    }
    Shader(const Shader&) = delete;             // non construction-copyable
    Shader& operator=(const Shader&) = delete;  // non copyable
    ~Shader() override { glDeleteShader(shader_id_); };
    GLuint GetShaderId() const override { return shader_id_; }

private:
    GLuint shader_id_{0};
};

using VertexShader = Shader<ShaderType::kVertexShader>;
using FragmentShader = Shader<ShaderType::kFragmentShader>;
using GeometryShader = Shader<ShaderType::kGeometryShader>;

}  // namespace nee::shader

#endif /* INCLUDE_SHADER_SHADER */
