#ifndef SCENE_HPP_INCLUDED
#define SCENE_HPP_INCLUDED

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

#include "Billboard.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "RenderQueue.hpp"
#include "SceneView.hpp"
#include "Skybox.hpp"

class Camera;
class Material;

struct MeshHandle {
    std::size_t index{};
};

struct MeshInstance {
    MeshHandle mesh;
    Transform transform;
    std::shared_ptr<Material> material;
    RenderLayer layer{RenderLayer::Opaque};
    float opacity{1.0F};
    bool animatedYaw{false};
};

struct BillboardInstance {
    Billboard billboard;
    glm::vec3 position{0.0F};
};

class Scene {
public:
    Scene() = default;
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    MeshHandle AddMesh(Mesh mesh);
    void AddMeshInstance(MeshHandle mesh, const Transform& transform,
                         std::shared_ptr<Material> material,
                         RenderLayer layer = RenderLayer::Opaque,
                         float opacity = 1.0F, bool animatedYaw = false);
    Billboard& AddBillboard(Billboard billboard, const glm::vec3& position);
    DirectionalLight& AddDirectionalLight(DirectionalLight light);
    PointLight& AddPointLight(PointLight light);
    SpotLight& AddSpotLight(SpotLight light);
    SkyBox& AddSkybox(SkyBox skybox);

    void SetAnimatedYaw(float degrees) noexcept;
    void Submit(RenderQueue& queue);
    [[nodiscard]] SceneView CreateView(const Camera& camera,
                                       const RenderQueue& queue);

    [[nodiscard]] std::span<DirectionalLight> GetDirectionalLights() noexcept;
    [[nodiscard]] std::span<PointLight> GetPointLights() noexcept;
    [[nodiscard]] std::span<SpotLight> GetSpotLights() noexcept;

private:
    std::vector<Mesh> meshes_;
    std::vector<MeshInstance> meshInstances_;
    std::vector<BillboardInstance> billboardInstances_;
    std::vector<DirectionalLight> directionalLights_;
    std::vector<PointLight> pointLights_;
    std::vector<SpotLight> spotLights_;
    std::vector<SkyBox> skyboxes_;
};

#endif  // SCENE_HPP_INCLUDED
