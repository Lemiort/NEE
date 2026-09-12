#ifndef SKYBOX_H_INCLUDED
#define SKYBOX_H_INCLUDED

#include <glad/gl.h>

#include <cstddef>
#include <memory>
#include <string>

#include "CubemapTexture.hpp"
// #include "ShaderFunctions.hpp"
#include "Camera.hpp"
#include "PlaceableObject.hpp"
#include "RenderableObject.hpp"
#include "Shader.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))
class SkyBox : public RenderableObject, public PlaceableObject {
public:
    explicit SkyBox(shared_ptr<Shader> shader);
    ~SkyBox() override;

    bool Init(const string& Directory, const string& PosXFilename,
              const string& NegXFilename, const string& PosYFilename,
              const string& NegYFilename, const string& PosZFilename,
              const string& NegZFilename);

    void Render(const Camera& cam) override;
    GLuint shaderProgramID;
    GLuint VBO;  // vertex buffer, ������ ������� ��� ���������
    GLuint positionID;
    GLuint IBO;  // index buffer
    GLuint gWorldID, gCamViewID;
    int spfaces, spverts;
    int Scale;
    void Init(GLuint shader, const char* model);

private:
    std::unique_ptr<CubemapTexture> pCubemapTex;
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

#endif  // SKYBOX_H_INCLUDED
