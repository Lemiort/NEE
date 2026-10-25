#include "DeferredPipeline.hpp"

#include <array>
#include <functional>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <optional>
#include <utility>

#include "ForwardPipeline.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "RenderQueue.hpp"

namespace {
void SetLightColor(GLint location, const glm::vec3& color) {
    const glm::mat4 matrix = glm::scale(glm::mat4(1.0F), color);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

Transform PointLightVolumeTransform(const PointLight& light) {
    return {.position = light.GetPosition(),
            .rotationDegrees = glm::vec3(0.0F),
            .scale = glm::vec3(light.GetRadius())};
}

std::optional<Transform> SpotLightVolumeTransform(const SpotLight& light) {
    const glm::vec3 position = light.GetPosition();
    const glm::vec3 direction = light.GetDirection();
    const float distance = glm::length(direction);
    if (distance <= 1e-6F) {
        return std::nullopt;
    }

    const glm::vec3 unit = direction / distance;
    const float x = glm::degrees(glm::asin(glm::clamp(unit.z, -1.0F, 1.0F)));
    const float z = glm::abs(unit.z) >= 1.0F - 1e-6F
                        ? 0.0F
                        : glm::degrees(glm::atan(-unit.x, unit.y));
    const float radius = distance * glm::tan(glm::radians(light.GetCutoff()));
    return Transform{.position = position,
                     .rotationDegrees = glm::vec3(x, 0.0F, z),
                     .scale = glm::vec3(radius, distance, radius)};
}
}  // namespace

DeferredPipeline::DeferredPipeline(DeferredPipelineResources resources)
    : gBuffer_(std::move(resources.gBuffer)),
      geometryShader_(std::move(resources.geometryShader)),
      stencilShader_(std::move(resources.stencilShader)),
      pointLightShader_(std::move(resources.pointLightShader)),
      spotLightShader_(std::move(resources.spotLightShader)),
      directionalLightShader_(std::move(resources.directionalLightShader)),
      geometryBindings_(MeshShaderBindings::FromShader(*geometryShader_)),
      stencilBindings_(MeshShaderBindings::FromShader(*stencilShader_)),
      pointLightBindings_(MeshShaderBindings::FromShader(*pointLightShader_)),
      spotLightBindings_(MeshShaderBindings::FromShader(*spotLightShader_)),
      stencilMaterial_(std::move(resources.stencilMaterial)),
      pointLightMaterial_(std::move(resources.pointLightMaterial)),
      spotLightMaterial_(std::move(resources.spotLightMaterial)),
      directionalLightMaterial_(std::move(resources.directionalLightMaterial)),
      pointLightVolume_(gsl_lite::not_null{pointLightMaterial_},
                        "models/normal_geosphere.obj"),
      spotLightVolume_(gsl_lite::not_null{spotLightMaterial_},
                       "models/cone.obj") {}

void DeferredPipeline::Render(const SceneView& scene,
                              ForwardPipeline& transparentPipeline) {
    gBuffer_->StartFrame();
    GeometryPass(scene);
    glEnable(GL_STENCIL_TEST);
    for (const PointLight& light : scene.pointLights) {
        StencilPass(light, scene.camera);
        PointLightPass(light, scene.camera);
    }
    for (const SpotLight& light : scene.spotLights) {
        StencilPass(light, scene.camera);
        SpotLightPass(light, scene.camera);
    }
    glDisable(GL_STENCIL_TEST);
    for (const DirectionalLight& light : scene.directionalLights) {
        DirectionalLightPass(light, scene.camera);
    }
    gBuffer_->BindForLightPass();
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    if (scene.skybox != nullptr) {
        scene.skybox->Draw(scene.camera);
    }
    transparentPipeline.RenderTransparent(scene);
    FinalPass(scene.camera);
}

void DeferredPipeline::RenderDebug(const SceneView& scene,
                                   GBufferDebugView view) {
    GeometryPass(scene);
    debugPass_.Render(*gBuffer_, scene.camera, view);
}

void DeferredPipeline::GeometryPass(const SceneView& scene) {
    gBuffer_->BindForGeomPass();
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    geometryShader_->Use();
    const glm::mat4 matrix = scene.camera.GetViewProjectionMatrix();
    glUniformMatrix4fv(geometryShader_->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(matrix));

    instanceRenderer_.Render(scene.queue.GetOpaqueMeshes(), geometryBindings_,
                             geometryShader_.get());
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}

void DeferredPipeline::StencilPass(const PointLight& light,
                                   const Camera& camera) {
    stencilShader_->Use();
    GBuffer::BindForStencilPass();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
    const glm::mat4 matrix = camera.GetViewProjectionMatrix();
    glUniformMatrix4fv(stencilShader_->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(matrix));
    pointLightVolume_.Draw(PointLightVolumeTransform(light), *stencilMaterial_,
                           stencilBindings_);
}

void DeferredPipeline::StencilPass(const SpotLight& light,
                                   const Camera& camera) {
    stencilShader_->Use();
    GBuffer::BindForStencilPass();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
    const glm::mat4 matrix = camera.GetViewProjectionMatrix();
    glUniformMatrix4fv(stencilShader_->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(matrix));
    const std::optional<Transform> transform = SpotLightVolumeTransform(light);
    if (transform) {
        spotLightVolume_.Draw(*transform, *stencilMaterial_, stencilBindings_);
    }
}

void DeferredPipeline::BindGBufferTextures(Material& material) {
    for (unsigned index = 0; index < 5; ++index) {
        material.SetTexture(gBuffer_->GetTexture(index), index + 4);
    }
}

void DeferredPipeline::PointLightPass(const PointLight& light,
                                      const Camera& camera) {
    gBuffer_->BindForLightPass();
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    pointLightShader_->Use();
    const glm::mat4 matrix = camera.GetViewProjectionMatrix();
    glUniformMatrix4fv(pointLightShader_->GetUniformLocation("gVC"), 1,
                       GL_FALSE, glm::value_ptr(matrix));
    glUniform3f(pointLightShader_->GetUniformLocation("s_vCamPos"),
                camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
    BindGBufferTextures(*pointLightMaterial_);
    SetLightColor(pointLightShader_->GetUniformLocation("pointLightColor"),
                  light.GetColor());
    const glm::vec3 position = light.GetPosition();
    glUniform3f(pointLightShader_->GetUniformLocation("s_vPointLightPos"),
                position.x, position.y, position.z);
    glUniform1f(pointLightShader_->GetUniformLocation("pointLightIntensity"),
                light.GetPower());
    pointLightVolume_.Draw(PointLightVolumeTransform(light),
                           *pointLightMaterial_, pointLightBindings_);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}

void DeferredPipeline::SpotLightPass(const SpotLight& light,
                                     const Camera& camera) {
    gBuffer_->BindForLightPass();
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    spotLightShader_->Use();
    const glm::mat4 matrix = camera.GetViewProjectionMatrix();
    glUniformMatrix4fv(spotLightShader_->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(matrix));
    glUniform3f(spotLightShader_->GetUniformLocation("s_vCamPos"),
                camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
    BindGBufferTextures(*spotLightMaterial_);
    const glm::vec3 position = light.GetPosition();
    glUniform3f(spotLightShader_->GetUniformLocation("sLightPos"), position.x,
                position.y, position.z);
    SetLightColor(spotLightShader_->GetUniformLocation("sLightCol"),
                  light.GetColor());
    const glm::vec3 direction = light.GetDirection();
    glUniform3f(spotLightShader_->GetUniformLocation("sLightDir"), direction.x,
                direction.y, direction.z);
    glUniform1f(spotLightShader_->GetUniformLocation("sLightCutoff"),
                glm::cos(glm::radians(light.GetCutoff())));
    const std::optional<Transform> transform = SpotLightVolumeTransform(light);
    if (transform) {
        spotLightVolume_.Draw(*transform, *spotLightMaterial_,
                              spotLightBindings_);
    }
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}

void DeferredPipeline::DirectionalLightPass(const DirectionalLight& light,
                                            const Camera& camera) {
    gBuffer_->BindForLightPass();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    BindGBufferTextures(*directionalLightMaterial_);
    directionalLightMaterial_->Use();
    glUniform3f(directionalLightShader_->GetUniformLocation("s_vCamPos"),
                camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
    SetLightColor(directionalLightShader_->GetUniformLocation("dirLightColor"),
                  light.GetColor());
    const glm::vec3 direction = light.GetDirection();
    glUniform3f(
        directionalLightShader_->GetUniformLocation("dirLightDirection"),
        direction.x, direction.y, direction.z);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

void DeferredPipeline::FinalPass(const Camera& camera) {
    gBuffer_->BindForFinalPass();
    glBlitFramebuffer(0, 0, camera.GetWidth(), camera.GetHeight(), 0, 0,
                      camera.GetWidth(), camera.GetHeight(),
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
