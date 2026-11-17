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

    // A pass shader overrides shader selection while each command retains its
    // surface material and textures. With a frustum, commands whose mesh
    // bounds lie outside it are skipped. Counts go to `statistics`.
    void Render(std::span<const MeshRenderCommand> commands,
                const MeshShaderBindings& bindings,
                Shader* passShader = nullptr, const Frustum* frustum = nullptr,
                RenderStatistics* statistics = nullptr);
    // Draws the batches inside the frustum, or all of them without one.
    static void RenderBatches(
        std::span<const std::reference_wrapper<const MeshBatch>> batches,
        const MeshShaderBindings& bindings, Shader* passShader = nullptr,
        const Frustum* frustum = nullptr,
        RenderStatistics* statistics = nullptr);

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
