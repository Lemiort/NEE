#include "ForwardPipeline.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

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

ForwardPipeline::ForwardPipeline(
    gsl_lite::not_null<std::shared_ptr<Shader>> meshShader)
    : meshShader_(std::move(meshShader)) {
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
}

void ForwardPipeline::Render(const SceneView& scene) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene.skybox.Render(scene.camera);
    PrepareLighting(scene);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    RenderMeshes(scene.queue.GetOpaqueMeshes(), scene.camera, false);
    RenderTransparentObjects(scene);
}

void ForwardPipeline::RenderTransparent(const SceneView& scene) {
    PrepareLighting(scene);
    RenderTransparentObjects(scene);
}

void ForwardPipeline::PrepareLighting(const SceneView& scene) {
    if (scene.pointLights.empty() || scene.spotLights.empty()) {
        return;
    }
    const PointLight& pointLight = scene.pointLights.front();
    const SpotLight& spotLight = scene.spotLights.front();

    const Camera lightCamera(scene.camera.GetWidth(), scene.camera.GetHeight(),
                             scene.camera.GetFov(), scene.camera.GetZNear(),
                             scene.camera.GetZFar(), spotLight.GetPosition(),
                             glm::vec3(-1.0F, -1.0F, -1.0F),
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
    SetLightColor(pointLightColorUniform_, pointLight.color);
    const glm::vec3 pointPosition = pointLight.GetPosition();
    glUniform3f(pointLightPositionUniform_, pointPosition.x, pointPosition.y,
                pointPosition.z);
    glUniform1f(pointLightIntensityUniform_, pointLight.power);
    SetLightColor(spotLightColorUniform_, spotLight.color);
    glUniform3f(spotLightDirectionUniform_, spotLight.direction.at(0),
                spotLight.direction.at(1), spotLight.direction.at(2));
    glUniform1f(spotLightCutoffUniform_,
                glm::cos(glm::radians(spotLight.Cutoff)));
    const glm::vec3 spotPosition = spotLight.GetPosition();
    glUniform3f(spotLightPositionUniform_, spotPosition.x, spotPosition.y,
                spotPosition.z);
    glUniform3f(cameraPositionUniform_, scene.camera.GetPos().x,
                scene.camera.GetPos().y, scene.camera.GetPos().z);
}

void ForwardPipeline::RenderMeshes(std::span<const MeshRenderCommand> commands,
                                   const Camera& camera, bool applyOpacity) {
    std::optional<std::reference_wrapper<Mesh>> previousMesh;
    std::optional<std::reference_wrapper<Material>> previousMaterial;
    for (const MeshRenderCommand& command : commands) {
        Mesh& mesh = command.mesh.get();
        if (command.material != nullptr &&
            (!previousMesh || &mesh != &previousMesh->get() ||
             !previousMaterial ||
             command.material.get() != &previousMaterial->get())) {
            mesh.SetMaterial(command.material);
            previousMesh = mesh;
            previousMaterial = *command.material;
        }
        if (command.material != nullptr) {
            command.material->SetOpacity(applyOpacity ? command.opacity : 1.0F);
        }
        mesh.SetTransform(command.transform);
        mesh.Render(camera);
    }
}

void ForwardPipeline::RenderTransparentObjects(const SceneView& scene) {
    using TransparentCommand =
        std::variant<std::reference_wrapper<const MeshRenderCommand>,
                     std::reference_wrapper<const BillboardRenderCommand>>;
    std::vector<TransparentCommand> sortedCommands;
    sortedCommands.reserve(scene.queue.GetTransparentMeshes().size() +
                           scene.queue.GetBillboards().size());
    for (const MeshRenderCommand& command :
         scene.queue.GetTransparentMeshes()) {
        sortedCommands.emplace_back(command);
    }
    for (const BillboardRenderCommand& command : scene.queue.GetBillboards()) {
        sortedCommands.emplace_back(command);
    }
    const glm::vec3 cameraPosition = scene.camera.GetPos();
    std::ranges::stable_sort(
        sortedCommands, std::greater{},
        [cameraPosition](const TransparentCommand& command) {
            const glm::vec3 position = std::visit(
                [](const auto value) {
                    const auto& referenced = value.get();
                    using Command = std::remove_cvref_t<decltype(referenced)>;
                    if constexpr (std::same_as<Command, MeshRenderCommand>) {
                        return referenced.transform.position;
                    } else {
                        return referenced.position;
                    }
                },
                command);
            const glm::vec3 offset = position - cameraPosition;
            return glm::dot(offset, offset);
        });

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const TransparentCommand& command : sortedCommands) {
        std::visit(
            [this, &scene](const auto value) {
                const auto& referenced = value.get();
                using Command = std::remove_cvref_t<decltype(referenced)>;
                if constexpr (std::same_as<Command, MeshRenderCommand>) {
                    RenderMeshes(
                        std::span<const MeshRenderCommand>(&referenced, 1),
                        scene.camera, true);
                } else {
                    referenced.billboard.get().SetPos(referenced.position);
                    referenced.billboard.get().Render(scene.camera);
                }
            },
            command);
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}
