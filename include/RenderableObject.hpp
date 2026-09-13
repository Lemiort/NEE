#ifndef RENDERABLEOBJECT_H
#define RENDERABLEOBJECT_H

#include <memory>

#include "Camera.hpp"
#include "Object.hpp"
#include "Shader.hpp"

// Renderable Objects can be rendered, needs Shader to render
class RenderableObject : public Object {
public:
    RenderableObject();
    virtual ~RenderableObject();
    virtual void Render(const Camera& cam);
    virtual std::shared_ptr<Shader> GetShader();
    virtual void SetShader(std::shared_ptr<Shader> shader);

protected:
    std::shared_ptr<Shader> shaderProgram;
    GLuint VBO, IBO;

private:
};

#endif  // RENDERABLEOBJECT_H
