#ifndef BILLBOARD_H
#define BILLBOARD_H
#include <glm/glm.hpp>

#include "Assistant.h"
#include "Camera.h"
#include "RenderableObject.h"
#include "Shader.h"
#include "ShaderFunctions.h"
#include "Texture.h"
class Billboard : public RenderableObject {
public:
    Billboard();
    //  Billboard(GLuint shader);
    Billboard(shared_ptr<Shader> shader);
    ~Billboard();
    void Init(const char* TexFilename);
    void Render(Camera* cam);
    void SetPos(glm::vec3 _Pos);
    // GLuint shaderProgramID;
protected:
private:
    // shared_ptr<Shader> shaderProgram;
    bool shader;
    GLuint VBO;
    glm::vec3 Pos;
    Texture2D colorMap;
    GLuint colSamplerID;
    GLuint positionID;
    GLuint camViewID, camPosID;
};

#endif  // BILLBOARD_H
