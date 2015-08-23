#include "RenderableObject.h"

RenderableObject::RenderableObject()
{
    //ctor
}

RenderableObject::~RenderableObject()
{
    //dtor
}


void RenderableObject::Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam)
{

}


Shader* RenderableObject::GetShader()
{
    return shaderProgram;
}
void RenderableObject::SetShader(Shader* shader)
{
    shaderProgram = shader;
}
