#ifndef INCLUDE_SKYBOX_H
#define INCLUDE_SKYBOX_H

#include <memory>
#include <string>

#include "Assistant.h"
#include "Camera.h"
#include "PlaceableObject.h"
#include "RenderableObject.h"
#include "Shader.h"
#include "cubemap_texture.h"

class SkyBox : public RenderableObject, public PlaceableObject {
public:
    explicit SkyBox(std::shared_ptr<Shader> shader);
    ~SkyBox();

    bool Init(const std::string& filename);

    void Render(Camera* cam);
    GLuint shaderProgramID;
    GLuint VBO;  // vertex buffer
    GLuint positionID;
    GLuint IBO;  // index buffer
    GLuint gWorldID, gCamViewID;
    int spfaces, spverts;
    int Scale;
    void Init(GLuint shader, const char* model);

private:
    const Camera* pCamera;
    CubemapTexture* pCubemapTex;
    GLuint WVPID;
    GLuint textureID;
    // shared_ptr<Shader> shaderProgram;
public:
    GLuint uvID, normalID;
    GLuint rotateID;
    GLuint camtransID, camPosID;
    GLuint colTexID, texBufferID;

    // float position[3];
    float rotation[3];
    float scale[3];
};

#endif  // INCLUDE_SKYBOX_H
