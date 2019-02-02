#ifndef RENDERABLEOBJECT_H
#define RENDERABLEOBJECT_H

#include <Camera.h>
#include <Object.h>
#include <memory>
#include "Shader/Shader.h"

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

#endif  // RENDERABLEOBJECT_H
