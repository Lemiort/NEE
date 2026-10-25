#ifndef MESH_INSTANCE_RENDERER_HPP_INCLUDED
#define MESH_INSTANCE_RENDERER_HPP_INCLUDED

#include <span>
#include <vector>

#include "Mesh.hpp"
#include "RenderQueue.hpp"

// Owns streaming GPU storage; scene commands and meshes remain borrowed for a
// frame.
class MeshInstanceRenderer {
public:
    MeshInstanceRenderer() = default;
    ~MeshInstanceRenderer();
    MeshInstanceRenderer(const MeshInstanceRenderer&) = delete;
    MeshInstanceRenderer& operator=(const MeshInstanceRenderer&) = delete;

    // A pass shader overrides shader selection while each command retains its
    // surface material and textures.
    void Render(std::span<const MeshRenderCommand> commands,
                const MeshShaderBindings& bindings,
                Shader* passShader = nullptr);

private:
    struct InstanceData {
        glm::mat4 world;
        glm::mat4 normal;
    };
    GLuint buffer_{};
    std::vector<const MeshRenderCommand*> sorted_;
    std::vector<InstanceData> instances_;
};

#endif
