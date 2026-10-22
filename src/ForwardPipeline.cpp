#include "ForwardPipeline.hpp"

#include <array>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <utility>

#include "Billboard.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

namespace {
glm::mat4 RenderPassViewProjection(const Camera& camera) {
    const glm::mat4 projection =
        glm::perspectiveRH_NO(glm::radians(30.0F),
                              static_cast<float>(camera.GetWidth()) /
                                  static_cast<float>(camera.GetHeight()),
                              1.0F, 1000.0F);
    return projection * camera.GetViewMatrix();
}

void SetLightColor(GLint uniformLocation, const std::array<GLfloat, 3>& color) {
    const glm::mat4 colorMatrix = glm::scale(
        glm::mat4(1.0F), glm::vec3(color.at(0), color.at(1), color.at(2)));
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE,
                       glm::value_ptr(colorMatrix));
}
}  // namespace

bool ForwardPipeline::Init(std::shared_ptr<Shader> meshShader) {
    meshShader_ = std::move(meshShader);
    if (meshShader_ == nullptr) {
        return false;
    }

    cameraViewProjectionUniform_ = meshShader_->GetUniformLocation("gVC");
    lightViewProjectionUniform_ = meshShader_->GetUniformLocation("gLightVC");
    cameraPositionUniform_ = meshShader_->GetUniformLocation("s_vCamPos");
    directionalLightDirectionUniform_ =
        meshShader_->GetUniformLocation("dLightDir");
    directionalLightColorUniform_ =
        meshShader_->GetUniformLocation("dLightCol");
    pointLightPositionUniform_ = meshShader_->GetUniformLocation("pLightPos");
    pointLightColorUniform_ = meshShader_->GetUniformLocation("pLightCol");
    pointLightIntensityUniform_ = meshShader_->GetUniformLocation("pLightInt");
    spotLightPositionUniform_ = meshShader_->GetUniformLocation("sLightPos");
    spotLightDirectionUniform_ = meshShader_->GetUniformLocation("sLightDir");
    spotLightColorUniform_ = meshShader_->GetUniformLocation("sLightCol");
    spotLightCutoffUniform_ = meshShader_->GetUniformLocation("sLightCutoff");
    return true;
}

void ForwardPipeline::Render(const ForwardPipelineScene& scene) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene.skybox.Render(scene.camera);

    const Camera lightCamera(
        scene.camera.GetWidth(), scene.camera.GetHeight(),
        scene.camera.GetFov(), scene.camera.GetZNear(), scene.camera.GetZFar(),
        scene.spotLight.GetPosition(), glm::vec3(-1.0F, -1.0F, -1.0F),
        glm::vec3(0.0F, 1.0F, 0.0F));

    meshShader_->Use();
    const glm::mat4 viewProjection = RenderPassViewProjection(scene.camera);
    const glm::mat4 lightViewProjection = RenderPassViewProjection(lightCamera);
    glUniformMatrix4fv(cameraViewProjectionUniform_, 1, GL_FALSE,
                       glm::value_ptr(viewProjection));
    glUniformMatrix4fv(lightViewProjectionUniform_, 1, GL_FALSE,
                       glm::value_ptr(lightViewProjection));

    SetLightColor(directionalLightColorUniform_, scene.directionalLight.color);
    glUniform3f(directionalLightDirectionUniform_,
                scene.directionalLight.direction.at(0),
                scene.directionalLight.direction.at(1),
                scene.directionalLight.direction.at(2));
    SetLightColor(pointLightColorUniform_, scene.pointLight.color);
    const glm::vec3 pointPosition = scene.pointLight.GetPosition();
    glUniform3f(pointLightPositionUniform_, pointPosition.x, pointPosition.y,
                pointPosition.z);
    glUniform1f(pointLightIntensityUniform_, scene.pointLight.power);
    SetLightColor(spotLightColorUniform_, scene.spotLight.color);
    glUniform3f(spotLightDirectionUniform_, scene.spotLight.direction.at(0),
                scene.spotLight.direction.at(1),
                scene.spotLight.direction.at(2));
    glUniform1f(spotLightCutoffUniform_,
                glm::cos(glm::radians(scene.spotLight.Cutoff)));
    const glm::vec3 spotPosition = scene.spotLight.GetPosition();
    glUniform3f(spotLightPositionUniform_, spotPosition.x, spotPosition.y,
                spotPosition.z);
    glUniform3f(cameraPositionUniform_, scene.camera.GetPos().x,
                scene.camera.GetPos().y, scene.camera.GetPos().z);

    RenderMeshes(scene.queue.GetOpaqueMeshes(), scene.camera);
    RenderMeshes(scene.queue.GetTransparentMeshes(), scene.camera);

    for (const BillboardRenderCommand& command : scene.queue.GetBillboards()) {
        command.billboard.get().SetPos(command.position);
        command.billboard.get().Render(scene.camera);
    }
}

void ForwardPipeline::RenderMeshes(std::span<const MeshRenderCommand> commands,
                                   const Camera& camera) {
    Mesh* previousMesh = nullptr;
    Material* previousMaterial = nullptr;
    for (const MeshRenderCommand& command : commands) {
        Mesh& mesh = command.mesh.get();
        if (command.material != nullptr &&
            (&mesh != previousMesh ||
             command.material.get() != previousMaterial)) {
            mesh.SetMaterial(command.material);
            previousMesh = &mesh;
            previousMaterial = command.material.get();
        }
        mesh.SetTransform(command.transform);
        mesh.Render(camera);
    }
}
