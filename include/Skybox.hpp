#ifndef SKYBOX_H_INCLUDED
#define SKYBOX_H_INCLUDED

#include <glad/gl.h>

#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <string>

#include "Camera.hpp"
#include "CubemapTexture.hpp"
#include "RenderableObject.hpp"
#include "Shader.hpp"
#include "Transformable.hpp"

class SkyBox : public RenderableObject, public Transformable {
public:
    SkyBox(gsl_lite::not_null<std::shared_ptr<Shader>> shader,
           const std::string& directory, const std::string& positiveX,
           const std::string& negativeX, const std::string& positiveY,
           const std::string& negativeY, const std::string& positiveZ,
           const std::string& negativeZ);
    ~SkyBox() override;
    SkyBox(SkyBox&&) noexcept = default;
    SkyBox& operator=(SkyBox&&) noexcept = default;

    void Render(const Camera& cam) override;
    void Draw(const Camera& camera) const;

    GLuint shaderProgramID{};
    GLint positionID{};
    GLint gWorldID{}, gCamViewID{};
    int spfaces{}, spverts{};
    int Scale{};

private:
    std::unique_ptr<CubemapTexture> pCubemapTex;
    GLint WVPID{};
    GLint textureID{};

public:
    GLint uvID{}, normalID{};
    GLint rotateID{};
    GLint camtransID{}, camPosID{};
    GLint colTexID{};
    GLuint texBufferID{};
};

#endif  // SKYBOX_H_INCLUDED
