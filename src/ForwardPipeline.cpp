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

void SetLightColor(GLint uniformLocation, const glm::vec3& color) {
    const glm::mat4 colorMatrix = glm::scale(glm::mat4(1.0F), color);
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE,
                       glm::value_ptr(colorMatrix));
}
}  // namespace

ForwardPipeline::ForwardPipeline(
    gsl_lite::not_null<std::shared_ptr<Shader>> meshShader)
    : meshShader_(std::move(meshShader)),
      meshBindings_(MeshShaderBindings::FromShader(*meshShader_)) {
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
    if (scene.skybox != nullptr) {
        scene.skybox->Draw(scene.camera);
    }
    PrepareLighting(scene);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    RenderMeshes(scene.queue.GetOpaqueMeshes(), false);
    RenderTransparentObjects(scene);
}

void ForwardPipeline::RenderTransparent(const SceneView& scene) {
    PrepareLighting(scene);
    RenderTransparentObjects(scene);
}

void ForwardPipeline::PrepareLighting(const SceneView& scene) {
    meshShader_->Use();
    const glm::mat4 viewProjection = RenderPassViewProjection(scene.camera);
    glUniformMatrix4fv(cameraViewProjectionUniform_, 1, GL_FALSE,
                       glm::value_ptr(viewProjection));

    if (!scene.directionalLights.empty()) {
        const DirectionalLight& directionalLight =
            scene.directionalLights.front();
        SetLightColor(directionalLightColorUniform_,
                      directionalLight.GetColor());
        const glm::vec3 direction = directionalLight.GetDirection();
        glUniform3f(directionalLightDirectionUniform_, direction.x, direction.y,
                    direction.z);
    } else {
        constexpr glm::vec3 noLight{0.0F};
        SetLightColor(directionalLightColorUniform_, noLight);
    }

    if (!scene.pointLights.empty()) {
        const PointLight& pointLight = scene.pointLights.front();
        SetLightColor(pointLightColorUniform_, pointLight.GetColor());
        const glm::vec3 pointPosition = pointLight.GetPosition();
        glUniform3f(pointLightPositionUniform_, pointPosition.x,
                    pointPosition.y, pointPosition.z);
        glUniform1f(pointLightIntensityUniform_, pointLight.GetPower());
    } else {
        glUniform1f(pointLightIntensityUniform_, 0.0F);
    }

    if (!scene.spotLights.empty()) {
        const SpotLight& spotLight = scene.spotLights.front();
        const glm::vec3 spotDirection = spotLight.GetDirection();
        SetLightColor(spotLightColorUniform_, spotLight.GetColor());
        glUniform3f(spotLightDirectionUniform_, spotDirection.x,
                    spotDirection.y, spotDirection.z);
        glUniform1f(spotLightCutoffUniform_,
                    glm::cos(glm::radians(spotLight.GetCutoff())));
        const glm::vec3 spotPosition = spotLight.GetPosition();
        glUniform3f(spotLightPositionUniform_, spotPosition.x, spotPosition.y,
                    spotPosition.z);

        const Camera lightCamera(scene.camera.GetWidth(),
                                 scene.camera.GetHeight(),
                                 scene.camera.GetFov(), scene.camera.GetZNear(),
                                 scene.camera.GetZFar(), spotPosition,
                                 spotDirection, glm::vec3(0.0F, 1.0F, 0.0F));
        const glm::mat4 lightViewProjection =
            RenderPassViewProjection(lightCamera);
        glUniformMatrix4fv(lightViewProjectionUniform_, 1, GL_FALSE,
                           glm::value_ptr(lightViewProjection));
    } else {
        constexpr glm::vec3 noLight{0.0F};
        SetLightColor(spotLightColorUniform_, noLight);
    }
    glUniform3f(cameraPositionUniform_, scene.camera.GetPos().x,
                scene.camera.GetPos().y, scene.camera.GetPos().z);
}

void ForwardPipeline::RenderMeshes(std::span<const MeshRenderCommand> commands,
                                   bool applyOpacity) {
    for (const MeshRenderCommand& command : commands) {
        if (command.material != nullptr) {
            command.mesh.get().Draw(command.transform, *command.material,
                                    meshBindings_,
                                    applyOpacity ? command.opacity : 1.0F);
        }
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
                        true);
                } else {
                    referenced.billboard.get().Draw(scene.camera,
                                                    referenced.position);
                }
            },
            command);
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}
