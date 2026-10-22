#include "DeferredPipeline.hpp"

#include <array>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <utility>

#include "ForwardPipeline.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "RenderQueue.hpp"

namespace {
glm::mat4 RenderPassViewProjection(const Camera& camera) {
    const glm::mat4 projection =
        glm::perspectiveRH_NO(glm::radians(30.0F),
                              static_cast<float>(camera.GetWidth()) /
                                  static_cast<float>(camera.GetHeight()),
                              1.0F, 1000.0F);
    return projection * camera.GetViewMatrix();
}

void SetLightColor(GLint location, const std::array<GLfloat, 3>& color) {
    const glm::mat4 matrix = glm::scale(
        glm::mat4(1.0F), glm::vec3(color.at(0), color.at(1), color.at(2)));
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}
}  // namespace

bool DeferredPipeline::Init(DeferredPipelineResources resources) {
    gBuffer_ = &resources.gBuffer;
    geometryShader_ = std::move(resources.geometryShader);
    stencilShader_ = std::move(resources.stencilShader);
    pointLightShader_ = std::move(resources.pointLightShader);
    spotLightShader_ = std::move(resources.spotLightShader);
    directionalLightShader_ = std::move(resources.directionalLightShader);
    geometryMaterial_ = std::move(resources.geometryMaterial);
    stencilMaterial_ = std::move(resources.stencilMaterial);
    pointLightMaterial_ = std::move(resources.pointLightMaterial);
    spotLightMaterial_ = std::move(resources.spotLightMaterial);
    directionalLightMaterial_ = std::move(resources.directionalLightMaterial);
    return gBuffer_ != nullptr && geometryShader_ && stencilShader_ &&
           pointLightShader_ && spotLightShader_ && directionalLightShader_ &&
           geometryMaterial_ && stencilMaterial_ && pointLightMaterial_ &&
           spotLightMaterial_ && directionalLightMaterial_;
}

void DeferredPipeline::Render(const SceneView& scene,
                              ForwardPipeline& transparentPipeline) {
    gBuffer_->StartFrame();
    GeometryPass(scene);
    glEnable(GL_STENCIL_TEST);
    for (PointLight* light : scene.pointLights) {
        StencilPass(*light, scene.camera);
        PointLightPass(*light, scene.camera);
    }
    for (SpotLight* light : scene.spotLights) {
        StencilPass(*light, scene.camera);
        SpotLightPass(*light, scene.camera);
    }
    glDisable(GL_STENCIL_TEST);
    DirectionalLightPass(scene.directionalLight, scene.camera);
    transparentPipeline.RenderTransparent(scene);
    FinalPass(scene.camera);
}

void DeferredPipeline::RenderDebug(const SceneView& scene,
                                   GBufferDebugView view) {
    GeometryPass(scene);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gBuffer_->BindForReading();
    const auto texture = [view] {
        switch (view) {
            case GBufferDebugView::Position:
                return GBuffer::GBUFFER_TEXTURE_TYPE_POSITION;
            case GBufferDebugView::Diffuse:
                return GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE;
            case GBufferDebugView::Normal:
                return GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL;
            case GBufferDebugView::TextureCoordinates:
                return GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD;
        }
        return GBuffer::GBUFFER_TEXTURE_TYPE_POSITION;
    }();
    GBuffer::SetReadBuffer(texture);
    glBlitFramebuffer(0, 0, scene.camera.GetWidth(), scene.camera.GetHeight(),
                      0, 0, scene.camera.GetWidth(), scene.camera.GetHeight(),
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void DeferredPipeline::GeometryPass(const SceneView& scene) {
    gBuffer_->BindForGeomPass();
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    geometryShader_->Use();
    const glm::mat4 matrix = RenderPassViewProjection(scene.camera);
    glUniformMatrix4fv(geometryShader_->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(matrix));

    Mesh* previousMesh = nullptr;
    for (const MeshRenderCommand& command : scene.queue.GetOpaqueMeshes()) {
        Mesh& mesh = command.mesh.get();
        if (&mesh != previousMesh) {
            mesh.SetMaterial(geometryMaterial_);
            previousMesh = &mesh;
        }
        mesh.SetTransform(command.transform);
        mesh.Render(scene.camera);
    }
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}

void DeferredPipeline::StencilPass(Light& light, const Camera& camera) {
    stencilShader_->Use();
    GBuffer::BindForStencilPass();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
    const glm::mat4 matrix = RenderPassViewProjection(camera);
    glUniformMatrix4fv(stencilShader_->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(matrix));
    light.SetMaterial(stencilMaterial_);
    light.Render(camera);
}

void DeferredPipeline::BindGBufferTextures(Material& material) {
    for (unsigned index = 0; index < 5; ++index) {
        material.SetTexture(gBuffer_->GetTexture(index), index + 4);
    }
}

void DeferredPipeline::PointLightPass(PointLight& light, const Camera& camera) {
    gBuffer_->BindForLightPass();
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    pointLightShader_->Use();
    const glm::mat4 matrix = RenderPassViewProjection(camera);
    glUniformMatrix4fv(pointLightShader_->GetUniformLocation("gVC"), 1,
                       GL_FALSE, glm::value_ptr(matrix));
    glUniform3f(pointLightShader_->GetUniformLocation("s_vCamPos"),
                camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
    BindGBufferTextures(*pointLightMaterial_);
    SetLightColor(pointLightShader_->GetUniformLocation("pointLightColor"),
                  light.color);
    const glm::vec3 position = light.GetPosition();
    glUniform3f(pointLightShader_->GetUniformLocation("s_vPointLightPos"),
                position.x, position.y, position.z);
    glUniform1f(pointLightShader_->GetUniformLocation("pointLightIntensity"),
                light.power);
    light.SetMaterial(pointLightMaterial_);
    light.Render(camera);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}

void DeferredPipeline::SpotLightPass(SpotLight& light, const Camera& camera) {
    gBuffer_->BindForLightPass();
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    spotLightShader_->Use();
    const glm::mat4 matrix = RenderPassViewProjection(camera);
    glUniformMatrix4fv(spotLightShader_->GetUniformLocation("gVC"), 1, GL_FALSE,
                       glm::value_ptr(matrix));
    glUniform3f(spotLightShader_->GetUniformLocation("s_vCamPos"),
                camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
    BindGBufferTextures(*spotLightMaterial_);
    const glm::vec3 position = light.GetPosition();
    glUniform3f(spotLightShader_->GetUniformLocation("sLightPos"), position.x,
                position.y, position.z);
    SetLightColor(spotLightShader_->GetUniformLocation("sLightCol"),
                  light.color);
    glUniform3f(spotLightShader_->GetUniformLocation("sLightDir"),
                light.direction.at(0), light.direction.at(1),
                light.direction.at(2));
    glUniform1f(spotLightShader_->GetUniformLocation("sLightCutoff"),
                glm::cos(glm::radians(light.Cutoff)));
    light.SetMaterial(spotLightMaterial_);
    light.Render(camera);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}

void DeferredPipeline::DirectionalLightPass(DirectionalLight& light,
                                            const Camera& camera) {
    gBuffer_->BindForLightPass();
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    directionalLightShader_->Use();
    const glm::mat4 matrix = RenderPassViewProjection(camera);
    glUniformMatrix4fv(directionalLightShader_->GetUniformLocation("gVC"), 1,
                       GL_FALSE, glm::value_ptr(matrix));
    glUniform3f(directionalLightShader_->GetUniformLocation("s_vCamPos"),
                camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
    SetLightColor(directionalLightShader_->GetUniformLocation("dirLightColor"),
                  light.color);
    glUniform3f(
        directionalLightShader_->GetUniformLocation("dirLightDirection"),
        light.direction.at(0), light.direction.at(1), light.direction.at(2));
    BindGBufferTextures(*directionalLightMaterial_);
    light.SetMaterial(directionalLightMaterial_);
    light.Render(camera);
    glDisable(GL_BLEND);
}

void DeferredPipeline::FinalPass(const Camera& camera) {
    gBuffer_->BindForFinalPass();
    glBlitFramebuffer(0, 0, camera.GetWidth(), camera.GetHeight(), 0, 0,
                      camera.GetWidth(), camera.GetHeight(),
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
