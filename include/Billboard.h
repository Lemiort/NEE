#ifndef BILLBOARD_H
#define BILLBOARD_H
#include <Texture.h>
#include <math_3d.h>
#include <ShaderFunctions.h>
#include "Shader.h"
#include "Camera.h"
#include "Assistant.h"
#include <RenderableObject.h>
class Billboard: public RenderableObject
{
    public:
        Billboard();
      //  Billboard(GLuint shader);
        Billboard(shared_ptr<Shader> shader);
        ~Billboard();
        void Init(const char* TexFilename);
        void Render(Camera* cam);
        void SetPos(Vector3f _Pos);
       // GLuint shaderProgramID;
    protected:
    private:
        //shared_ptr<Shader> shaderProgram;
        bool shader;
        GLuint VBO;
        Vector3f Pos;
        Texture2D colorMap;
        GLuint colSamplerID;
        GLuint positionID;
        GLuint camViewID, camPosID;
};

#endif // BILLBOARD_H
