#ifndef INCLUDE_SHADER_ISHADER
#define INCLUDE_SHADER_ISHADER

#include <GL/glew.h>

#include <string>

namespace nee::shader {

class IShader {
public:
    //  enum ShaderType {Vertex, Geometry, Fragmnet};
    IShader() = default;
    virtual ~IShader() = default;
    virtual GLuint GetShaderId() const = 0;
};

}  // namespace nee::shader

#endif /* INCLUDE_SHADER_ISHADER */
