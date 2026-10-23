#ifndef FORWARD_PIPELINE_HPP_INCLUDED
#define FORWARD_PIPELINE_HPP_INCLUDED

#include <glad/gl.h>

#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <span>

#include "Camera.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "MeshInstanceRenderer.hpp"
#include "RenderQueue.hpp"
#include "SceneView.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"

class ForwardPipeline {
public:
    explicit ForwardPipeline(
        gsl_lite::not_null<std::shared_ptr<Shader>> meshShader);
    void Render(const SceneView& scene);
    void RenderTransparent(const SceneView& scene);

private:
    void PrepareLighting(const SceneView& scene);
    void RenderMeshes(std::span<const MeshRenderCommand> commands,
                      bool applyOpacity);
    void RenderTransparentObjects(const SceneView& scene);

    gsl_lite::not_null<std::shared_ptr<Shader>> meshShader_;
    MeshShaderBindings meshBindings_;
    MeshInstanceRenderer instanceRenderer_;
    GLint cameraViewProjectionUniform_{};
    GLint lightViewProjectionUniform_{};
    GLint cameraPositionUniform_{};
    GLint directionalLightDirectionUniform_{};
    GLint directionalLightColorUniform_{};
    GLint pointLightPositionUniform_{};
    GLint pointLightColorUniform_{};
    GLint pointLightIntensityUniform_{};
    GLint spotLightPositionUniform_{};
    GLint spotLightDirectionUniform_{};
    GLint spotLightColorUniform_{};
    GLint spotLightCutoffUniform_{};
};

#endif  // FORWARD_PIPELINE_HPP_INCLUDED
