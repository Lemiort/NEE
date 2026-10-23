#ifndef SCENE_VIEW_HPP_INCLUDED
#define SCENE_VIEW_HPP_INCLUDED

#include <span>

class Camera;
class DirectionalLight;
class PointLight;
class RenderQueue;
class SkyBox;
class SpotLight;

// Non-owning description of one frame. Every rendering pipeline consumes the
// same scene shape and keeps its implementation-specific resources internally.
struct SceneView {
    const Camera& camera;
    DirectionalLight& directionalLight;
    std::span<PointLight> pointLights;
    std::span<SpotLight> spotLights;
    SkyBox& skybox;
    const RenderQueue& queue;
};

#endif  // SCENE_VIEW_HPP_INCLUDED
