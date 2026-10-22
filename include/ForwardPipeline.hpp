#ifndef FORWARD_PIPELINE_HPP_INCLUDED
#define FORWARD_PIPELINE_HPP_INCLUDED

#include <glad/gl.h>

#include <memory>
#include <span>

#include "Camera.hpp"
#include "Light.hpp"
#include "RenderQueue.hpp"
#include "SceneView.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"

class ForwardPipeline {
public:
    [[nodiscard]] bool Init(std::shared_ptr<Shader> meshShader);
    void Render(const SceneView& scene);
    void RenderTransparent(const SceneView& scene);

private:
    void PrepareLighting(const SceneView& scene);
    void RenderMeshes(std::span<const MeshRenderCommand> commands,
                      const Camera& camera, bool applyOpacity);
    void RenderTransparentObjects(const SceneView& scene);

    std::shared_ptr<Shader> meshShader_;
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
    GLint opacityUniform_{};
};

#endif  // FORWARD_PIPELINE_HPP_INCLUDED
