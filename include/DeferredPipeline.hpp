#ifndef DEFERRED_PIPELINE_HPP_INCLUDED
#define DEFERRED_PIPELINE_HPP_INCLUDED

#include <memory>

#include "GBuffer.hpp"
#include "GBufferDebugPass.hpp"
#include "Material.hpp"
#include "SceneView.hpp"
#include "Shader.hpp"

class Light;
class ForwardPipeline;

struct DeferredPipelineResources {
    GBuffer& gBuffer;
    std::shared_ptr<Shader> geometryShader;
    std::shared_ptr<Shader> stencilShader;
    std::shared_ptr<Shader> pointLightShader;
    std::shared_ptr<Shader> spotLightShader;
    std::shared_ptr<Shader> directionalLightShader;
    std::shared_ptr<Material> geometryMaterial;
    std::shared_ptr<Material> stencilMaterial;
    std::shared_ptr<Material> pointLightMaterial;
    std::shared_ptr<Material> spotLightMaterial;
    std::shared_ptr<Material> directionalLightMaterial;
};

class DeferredPipeline {
public:
    [[nodiscard]] bool Init(DeferredPipelineResources resources);
    void Render(const SceneView& scene, ForwardPipeline& transparentPipeline);
    void RenderDebug(const SceneView& scene, GBufferDebugView view);

private:
    void GeometryPass(const SceneView& scene);
    void StencilPass(Light& light, const Camera& camera);
    void PointLightPass(PointLight& light, const Camera& camera);
    void SpotLightPass(SpotLight& light, const Camera& camera);
    void DirectionalLightPass(DirectionalLight& light, const Camera& camera);
    void BindGBufferTextures(Material& material);
    void FinalPass(const Camera& camera);

    GBuffer* gBuffer_{};
    std::shared_ptr<Shader> geometryShader_;
    std::shared_ptr<Shader> stencilShader_;
    std::shared_ptr<Shader> pointLightShader_;
    std::shared_ptr<Shader> spotLightShader_;
    std::shared_ptr<Shader> directionalLightShader_;
    std::shared_ptr<Material> geometryMaterial_;
    std::shared_ptr<Material> stencilMaterial_;
    std::shared_ptr<Material> pointLightMaterial_;
    std::shared_ptr<Material> spotLightMaterial_;
    std::shared_ptr<Material> directionalLightMaterial_;
    GBufferDebugPass debugPass_;
};

#endif  // DEFERRED_PIPELINE_HPP_INCLUDED
