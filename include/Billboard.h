#ifndef INCLUDE_BILLBOARD_H
#define INCLUDE_BILLBOARD_H

#include <glm/glm.hpp>
#include <memory>

#include "Camera.h"
#include "RenderableObject.h"
#include "Shader.h"
#include "ShaderFunctions.h"
#include "Texture.h"
class Billboard : public RenderableObject {
public:
    Billboard();
    //  Billboard(GLuint shader);
    explicit Billboard(std::shared_ptr<Shader> shader);
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
    glm::vec3 pos;
    Texture2D colorMap;
    GLuint colSamplerID;
    GLuint position_id;
    GLuint camViewID, camPosID;
};

#endif  // INCLUDE_BILLBOARD_H
