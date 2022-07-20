#ifndef INCLUDE_RENDER_IRENDERABLE
#define INCLUDE_RENDER_IRENDERABLE
#include "Camera.h"
#include "Shader/IShaderProgram.hpp"

namespace nee::render {
class IRenderable {
public:
    virtual ~IRenderable() = default;
    virtual void Render(const Camera& cam, const& IShaderProgram shader) = 0;
};

}  // namespace nee::render

#endif /* INCLUDE_RENDER_IRENDERABLE */
