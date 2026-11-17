#ifndef MESH_INSTANCE_RENDERER_HPP_INCLUDED
#define MESH_INSTANCE_RENDERER_HPP_INCLUDED

#include <span>
#include <vector>

#include "Bounds.hpp"
#include "Mesh.hpp"
#include "MeshBatch.hpp"
#include "RenderQueue.hpp"
#include "RenderStatistics.hpp"

// Owns streaming GPU storage; scene commands and meshes remain borrowed for a
// frame.
class MeshInstanceRenderer {
public:
    MeshInstanceRenderer() = default;
    ~MeshInstanceRenderer();
    MeshInstanceRenderer(const MeshInstanceRenderer&) = delete;
    MeshInstanceRenderer& operator=(const MeshInstanceRenderer&) = delete;

    // Draws the commands whose mesh bounds lie inside `frustum` and returns
    // what it drew and skipped. A pass shader overrides shader selection
    // while each command retains its surface material and textures.
    RenderStatistics Render(std::span<const MeshRenderCommand> commands,
                            const MeshShaderBindings& bindings,
                            const Frustum& frustum,
                            Shader* passShader = nullptr);
    // Draws the batches inside `frustum` and returns what it drew and
    // skipped.
    static RenderStatistics RenderBatches(
        std::span<const std::reference_wrapper<const MeshBatch>> batches,
        const MeshShaderBindings& bindings, const Frustum& frustum,
        Shader* passShader = nullptr);

private:
    struct InstanceData {
        glm::mat4 world;
        glm::mat4 normal;
    };
    struct Entry {
        const MeshRenderCommand* command;
        glm::mat4 world;
    };
    GLuint buffer_{};
    std::vector<Entry> entries_;
    std::vector<InstanceData> instances_;
};

#endif
