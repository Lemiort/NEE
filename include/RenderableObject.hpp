#ifndef RENDERABLEOBJECT_H
#define RENDERABLEOBJECT_H

#include <glad/gl.h>

#include <memory>

#include "Camera.hpp"
#include "Shader.hpp"

// Renderable Objects can be rendered, needs Shader to render
class IRenderable {
public:
    virtual ~IRenderable() = default;
    virtual void Render(const Camera& camera) = 0;
};

class RenderableObject : public IRenderable {
public:
    RenderableObject();
    ~RenderableObject() override;
    RenderableObject(const RenderableObject&) = delete;
    RenderableObject& operator=(const RenderableObject&) = delete;
    RenderableObject(RenderableObject&& other) noexcept;
    RenderableObject& operator=(RenderableObject&& other) noexcept;
    void Render(const Camera& camera) override = 0;
    [[nodiscard]] virtual std::shared_ptr<Shader> GetShader() const;
    virtual void SetShader(std::shared_ptr<Shader> shader);

protected:
    std::shared_ptr<Shader> shaderProgram;
    GLuint VBO{}, IBO{};

private:
};

#endif  // RENDERABLEOBJECT_H
