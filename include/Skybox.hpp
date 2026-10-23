#ifndef SKYBOX_H_INCLUDED
#define SKYBOX_H_INCLUDED

#include <glad/gl.h>

#include <cstddef>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <string>

#include "Camera.hpp"
#include "CubemapTexture.hpp"
#include "RenderableObject.hpp"
#include "Shader.hpp"
#include "Transformable.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))
class SkyBox : public RenderableObject, public Transformable {
public:
    SkyBox(gsl_lite::not_null<shared_ptr<Shader>> shader,
           const string& directory, const string& positiveX,
           const string& negativeX, const string& positiveY,
           const string& negativeY, const string& positiveZ,
           const string& negativeZ);
    ~SkyBox() override;

    void Render(const Camera& cam) override;
    GLuint shaderProgramID;
    GLuint VBO{};  // vertex buffer, ������ ������� ��� ���������
    GLint positionID{};
    GLuint IBO{};  // index buffer
    GLint gWorldID{}, gCamViewID{};
    int spfaces{}, spverts{};
    int Scale{};

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
};

#endif  // SKYBOX_H_INCLUDED
