#include "Scene.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Billboard.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "Skybox.hpp"

static_assert(std::is_nothrow_move_constructible_v<Mesh>);
static_assert(std::is_nothrow_move_assignable_v<Mesh>);
static_assert(std::is_nothrow_move_constructible_v<Billboard>);
static_assert(std::is_nothrow_move_assignable_v<Billboard>);
static_assert(std::is_nothrow_move_constructible_v<SkyBox>);
static_assert(std::is_nothrow_move_assignable_v<SkyBox>);
static_assert(std::is_nothrow_move_constructible_v<DirectionalLight>);
static_assert(std::is_nothrow_move_assignable_v<DirectionalLight>);
static_assert(std::is_nothrow_move_constructible_v<PointLight>);
static_assert(std::is_nothrow_move_assignable_v<PointLight>);
static_assert(std::is_nothrow_move_constructible_v<SpotLight>);
static_assert(std::is_nothrow_move_assignable_v<SpotLight>);

Scene::~Scene() = default;

MeshHandle Scene::AddMesh(Mesh mesh) {
    meshes_.push_back(std::move(mesh));
    return {.index = meshes_.size() - 1};
}

void Scene::AddMeshInstance(MeshHandle mesh, const Transform& transform,
                            std::shared_ptr<Material> material,
                            RenderLayer layer, float opacity,
                            bool animatedYaw) {
    if (material == nullptr) {
        throw std::invalid_argument("A mesh instance requires a material");
    }
    if (layer == RenderLayer::Overlay) {
        throw std::invalid_argument(
            "A scene mesh must use the opaque or transparent render layer");
    }
    if (mesh.index >= meshes_.size()) {
        throw std::out_of_range("Mesh handle does not belong to this scene");
    }
    meshInstances_.push_back({.mesh = mesh,
                              .transform = transform,
                              .material = std::move(material),
                              .layer = layer,
                              .opacity = opacity,
                              .animatedYaw = animatedYaw});
}

Billboard& Scene::AddBillboard(Billboard billboard, const glm::vec3& position) {
    billboardInstances_.push_back(
        {.billboard = std::move(billboard), .position = position});
    return billboardInstances_.back().billboard;
}

DirectionalLight& Scene::AddDirectionalLight(DirectionalLight light) {
    directionalLights_.push_back(std::move(light));
    return directionalLights_.back();
}

PointLight& Scene::AddPointLight(PointLight light) {
    pointLights_.push_back(std::move(light));
    return pointLights_.back();
}

SpotLight& Scene::AddSpotLight(SpotLight light) {
    spotLights_.push_back(std::move(light));
    return spotLights_.back();
}

SkyBox& Scene::AddSkybox(SkyBox skybox) {
    skyboxes_.push_back(std::move(skybox));
    return skyboxes_.back();
}

void Scene::SetAnimatedYaw(float degrees) noexcept {
    for (MeshInstance& instance : meshInstances_) {
        if (instance.animatedYaw) {
            instance.transform.rotationDegrees.y = degrees;
        }
    }
}

void Scene::Submit(RenderQueue& queue) {
    for (const MeshInstance& instance : meshInstances_) {
        Mesh& mesh = meshes_.at(instance.mesh.index);
        if (instance.layer == RenderLayer::Transparent) {
            queue.SubmitTransparentMesh(mesh, instance.transform,
                                        instance.material, instance.opacity);
        } else {
            queue.SubmitOpaqueMesh(mesh, instance.transform, instance.material);
        }
    }
    for (BillboardInstance& instance : billboardInstances_) {
        queue.SubmitBillboard(instance.billboard, instance.position);
    }
}

SceneView Scene::CreateView(const Camera& camera, const RenderQueue& queue) {
    if (directionalLights_.empty() || skyboxes_.empty()) {
        throw std::logic_error(
            "A renderable scene requires a directional light and skybox");
    }
    return {.camera = camera,
            .directionalLight = directionalLights_.front(),
            .pointLights = pointLights_,
            .spotLights = spotLights_,
            .skybox = skyboxes_.front(),
            .queue = queue};
}

std::span<DirectionalLight> Scene::GetDirectionalLights() noexcept {
    return directionalLights_;
}

std::span<PointLight> Scene::GetPointLights() noexcept { return pointLights_; }

std::span<SpotLight> Scene::GetSpotLights() noexcept { return spotLights_; }
