#ifndef INCLUDE_RENDERABLEOBJECT_H
#define INCLUDE_RENDERABLEOBJECT_H

#include <memory>

#include "Camera.h"
#include "Object.h"
#include "Shader.h"

// Renderable Objects can be rendered, needs Shader to render
class RenderableObject : public Object {
public:
    RenderableObject() = default;
    virtual ~RenderableObject() = default;
    virtual void Render(const Camera& cam) = 0;
    virtual std::shared_ptr<Shader> GetShader();
    virtual void SetShader(std::shared_ptr<Shader> shader);

protected:
    std::shared_ptr<Shader> shaderProgram{};
    GLuint VBO{}, IBO{};

private:
};

#endif  // INCLUDE_RENDERABLEOBJECT_H
