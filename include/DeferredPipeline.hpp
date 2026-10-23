#ifndef DEFERRED_PIPELINE_HPP_INCLUDED
#define DEFERRED_PIPELINE_HPP_INCLUDED

#include <gsl-lite/gsl-lite.hpp>
#include <memory>

#include "GBuffer.hpp"
#include "GBufferDebugPass.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "SceneView.hpp"
#include "Shader.hpp"

class Light;
class ForwardPipeline;

struct DeferredPipelineResources {
    gsl_lite::not_null<std::shared_ptr<GBuffer>> gBuffer;
    gsl_lite::not_null<std::shared_ptr<Shader>> geometryShader;
    gsl_lite::not_null<std::shared_ptr<Shader>> stencilShader;
    gsl_lite::not_null<std::shared_ptr<Shader>> pointLightShader;
    gsl_lite::not_null<std::shared_ptr<Shader>> spotLightShader;
    gsl_lite::not_null<std::shared_ptr<Shader>> directionalLightShader;
    gsl_lite::not_null<std::shared_ptr<Material>> geometryMaterial;
    gsl_lite::not_null<std::shared_ptr<Material>> stencilMaterial;
    gsl_lite::not_null<std::shared_ptr<Material>> pointLightMaterial;
    gsl_lite::not_null<std::shared_ptr<Material>> spotLightMaterial;
    gsl_lite::not_null<std::shared_ptr<Material>> directionalLightMaterial;
};

class DeferredPipeline {
public:
    explicit DeferredPipeline(DeferredPipelineResources resources);
    void Render(const SceneView& scene, ForwardPipeline& transparentPipeline);
    void RenderDebug(const SceneView& scene, GBufferDebugView view);

private:
    void GeometryPass(const SceneView& scene);
    void StencilPass(const PointLight& light, const Camera& camera);
    void StencilPass(const SpotLight& light, const Camera& camera);
    void PointLightPass(const PointLight& light, const Camera& camera);
    void SpotLightPass(const SpotLight& light, const Camera& camera);
    void DirectionalLightPass(const DirectionalLight& light,
                              const Camera& camera);
    void BindGBufferTextures(Material& material);
    void FinalPass(const Camera& camera);

    gsl_lite::not_null<std::shared_ptr<GBuffer>> gBuffer_;
    gsl_lite::not_null<std::shared_ptr<Shader>> geometryShader_;
    gsl_lite::not_null<std::shared_ptr<Shader>> stencilShader_;
    gsl_lite::not_null<std::shared_ptr<Shader>> pointLightShader_;
    gsl_lite::not_null<std::shared_ptr<Shader>> spotLightShader_;
    gsl_lite::not_null<std::shared_ptr<Shader>> directionalLightShader_;
    MeshShaderBindings geometryBindings_;
    MeshShaderBindings stencilBindings_;
    MeshShaderBindings pointLightBindings_;
    MeshShaderBindings spotLightBindings_;
    MeshShaderBindings directionalLightBindings_;
    gsl_lite::not_null<std::shared_ptr<Material>> geometryMaterial_;
    gsl_lite::not_null<std::shared_ptr<Material>> stencilMaterial_;
    gsl_lite::not_null<std::shared_ptr<Material>> pointLightMaterial_;
    gsl_lite::not_null<std::shared_ptr<Material>> spotLightMaterial_;
    gsl_lite::not_null<std::shared_ptr<Material>> directionalLightMaterial_;
    GBufferDebugPass debugPass_;
};

#endif  // DEFERRED_PIPELINE_HPP_INCLUDED
