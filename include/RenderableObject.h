#ifndef RENDERABLEOBJECT_H
#define RENDERABLEOBJECT_H

#include <Object.h>
#include <Camera.h>
#include <Shader.h>

class RenderableObject : public Object
{
    public:
        RenderableObject();
        virtual ~RenderableObject();
        virtual void Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam);
        virtual Shader* GetShader();
        virtual void SetShader(Shader* shader);
    protected:
        Shader* shaderProgram;
    private:
};

#endif // RENDERABLEOBJECT_H
