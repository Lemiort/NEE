#ifndef BILLBOARD_H
#define BILLBOARD_H
#include <RenderableObject.h>
#include <ShaderFunctions.h>
#include <Texture.h>
#include <math_3d.h>
#include "Assistant.h"
#include "Camera.h"
#include "Shader.h"
class Billboard : public RenderableObject {
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
    // shared_ptr<Shader> shaderProgram;
    bool shader;
    GLuint VBO;
    Vector3f Pos;
    Texture2D colorMap;
    GLuint colSamplerID;
    GLuint positionID;
    GLuint camViewID, camPosID;
};

#endif  // BILLBOARD_H
