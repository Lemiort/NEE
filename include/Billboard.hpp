#ifndef BILLBOARD_H
#define BILLBOARD_H
#include "Assistant.hpp"
#include "Camera.hpp"
#include "Math3d.hpp"
#include "RenderableObject.hpp"
#include "Shader.hpp"
#include "ShaderFunctions.hpp"
#include "Texture.hpp"
class Billboard : public RenderableObject {
public:
    Billboard();
    //  Billboard(GLuint shader);
    Billboard(shared_ptr<Shader> shader);
    ~Billboard();
    void Init(const char* TexFilename);
    // Render using a reference instead of a raw pointer for safety.
    // Render using a reference instead of a raw pointer for safety.
    void Render(const Camera& cam);
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
