#ifndef SKYBOX_H_INCLUDED
#define SKYBOX_H_INCLUDED

#include <glad/gl.h>

#include <cstddef>
#include <array>
#include <memory>
#include <string>

#include "Camera.hpp"
#include "CubemapTexture.hpp"
#include "PlaceableObject.hpp"
#include "RenderableObject.hpp"
#include "Shader.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))
class SkyBox : public RenderableObject, public PlaceableObject {
public:
 explicit SkyBox(const shared_ptr<Shader>& shader);
 ~SkyBox() override;

 bool Init(const string& Directory, const string& PosXFilename,
           const string& NegXFilename, const string& PosYFilename,
           const string& NegYFilename, const string& PosZFilename,
           const string& NegZFilename);

 void Render(const Camera& cam) override;
 GLuint shaderProgramID;
 GLuint VBO{};  // vertex buffer, ������ ������� ��� ���������
 GLint positionID{};
 GLuint IBO{};  // index buffer
 GLint gWorldID{}, gCamViewID{};
 int spfaces{}, spverts{};
 int Scale{};
 void Init(GLuint shader, const char* model);

private:
    std::unique_ptr<CubemapTexture> pCubemapTex;
    GLint WVPID{};
    GLint textureID{};
    // shared_ptr<Shader> shaderProgram;
public:
 GLint uvID{}, normalID{};
 GLint rotateID{};
 GLint camtransID{}, camPosID{};
 GLint colTexID{};
 GLuint texBufferID{};

 // float position[3];
 std::array<float, 3> rotation{};
 std::array<float, 3> scale{};
};

#endif  // SKYBOX_H_INCLUDED
