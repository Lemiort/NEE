#ifndef DEFERRED_PIPELINE_HPP_INCLUDED
#define DEFERRED_PIPELINE_HPP_INCLUDED

#include <gsl-lite/gsl-lite.hpp>
#include <memory>

#include "GBuffer.hpp"
#include "GBufferDebugPass.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "MeshInstanceRenderer.hpp"
#include "SceneView.hpp"
#include "Shader.hpp"

class Light;
class ForwardPipeline;

// How the HDR light of a frame becomes screen colors: multiplied by
// `exposure`, then compressed by the operator. None only clamps to 1, which
// with an exposure of 1 is how frames looked before HDR.
enum class ToneMapOperator { None, Reinhard, Aces };

struct ToneMapping {
    ToneMapOperator op{ToneMapOperator::None};
    float exposure{1.0F};
};

struct DeferredPipelineResources {
    gsl_lite::not_null<std::shared_ptr<GBuffer>> gBuffer;
    gsl_lite::not_null<std::shared_ptr<Shader>> geometryShader;
    gsl_lite::not_null<std::shared_ptr<Shader>> stencilShader;
    gsl_lite::not_null<std::shared_ptr<Shader>> pointLightShader;
    gsl_lite::not_null<std::shared_ptr<Shader>> spotLightShader;
    gsl_lite::not_null<std::shared_ptr<Shader>> directionalLightShader;
    // shaders/tonemap.vsh and shaders/tonemap.fsh.
    gsl_lite::not_null<std::shared_ptr<Shader>> toneMapShader;
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
    void SetToneMapping(const ToneMapping& toneMapping) {
        toneMapping_ = toneMapping;
    }
    [[nodiscard]] const ToneMapping& GetToneMapping() const noexcept {
        return toneMapping_;
    }

private:
    void GeometryPass(const SceneView& scene);
    void StencilPass(const Mesh& volume, const Transform& transform,
                     const Camera& camera);
    void PointLightPass(const PointLight& light, const Transform& volume,
                        const Camera& camera);
    void SpotLightPass(const SpotLight& light, const Transform& volume,
                       const Camera& camera);
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
    gsl_lite::not_null<std::shared_ptr<Shader>> toneMapShader_;
    ToneMapping toneMapping_;
    MeshShaderBindings geometryBindings_;
    MeshShaderBindings stencilBindings_;
    MeshShaderBindings pointLightBindings_;
    MeshShaderBindings spotLightBindings_;
    gsl_lite::not_null<std::shared_ptr<Material>> stencilMaterial_;
    gsl_lite::not_null<std::shared_ptr<Material>> pointLightMaterial_;
    gsl_lite::not_null<std::shared_ptr<Material>> spotLightMaterial_;
    gsl_lite::not_null<std::shared_ptr<Material>> directionalLightMaterial_;
    Mesh pointLightVolume_;
    Mesh spotLightVolume_;
    GBufferDebugPass debugPass_;
    MeshInstanceRenderer instanceRenderer_;
    VertexArray fullscreenVertexArray_;
};

#endif  // DEFERRED_PIPELINE_HPP_INCLUDED
