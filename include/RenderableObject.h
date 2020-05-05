#ifndef INCLUDE_RENDERABLEOBJECT_H_
#define INCLUDE_RENDERABLEOBJECT_H_

#include <memory>

#include "Camera.h"
#include "Object.h"
#include "Shader.h"

// Renderable Objects can be rendered, needs Shader to render
class RenderableObject : public Object {
public:
    RenderableObject();
    virtual ~RenderableObject();
    virtual void Render(Camera* cam);
    virtual std::shared_ptr<Shader> GetShader();
    virtual void SetShader(std::shared_ptr<Shader> shader);

protected:
    std::shared_ptr<Shader> shaderProgram;
    GLuint VBO, IBO;

private:
};

#endif  // INCLUDE_RENDERABLEOBJECT_H_
