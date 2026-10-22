#ifndef RENDER_QUEUE_HPP_INCLUDED
#define RENDER_QUEUE_HPP_INCLUDED

#include <functional>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <span>
#include <vector>

#include "Text2D.hpp"
#include "Transform.hpp"

class Billboard;
class Material;
class Mesh;

enum class RenderLayer { Opaque, Transparent, Overlay };

// Commands reference low-level render objects, never game-domain objects.
// The queue does not own referenced meshes or billboards; their owners must
// keep them alive until the frame has been executed.
struct MeshRenderCommand {
    std::reference_wrapper<Mesh> mesh;
    Transform transform;
    std::shared_ptr<Material> material;
    RenderLayer layer{RenderLayer::Opaque};
    float opacity{1.0F};
};

struct BillboardRenderCommand {
    std::reference_wrapper<Billboard> billboard;
    glm::vec3 position{0.0F};
    RenderLayer layer{RenderLayer::Transparent};
};

struct TextRenderCommand {
    FontLine2d text;
    RenderLayer layer{RenderLayer::Overlay};
};

struct DebugLineRenderCommand {
    glm::vec3 start{0.0F};
    glm::vec3 end{0.0F};
    glm::vec3 color{1.0F};
    RenderLayer layer{RenderLayer::Overlay};
};

class RenderQueue {
public:
    void SubmitOpaqueMesh(Mesh& mesh, const Transform& transform,
                          std::shared_ptr<Material> material);
    void SubmitTransparentMesh(Mesh& mesh, const Transform& transform,
                               std::shared_ptr<Material> material,
                               float opacity);
    void SubmitBillboard(Billboard& billboard, const glm::vec3& position);
    void SubmitText(FontLine2d text);
    void SubmitDebugLine(const DebugLineRenderCommand& line);

    void Clear() noexcept;
    void ClearText() noexcept;

    [[nodiscard]] std::span<const MeshRenderCommand> GetOpaqueMeshes()
        const noexcept;
    [[nodiscard]] std::span<const MeshRenderCommand> GetTransparentMeshes()
        const noexcept;
    [[nodiscard]] std::span<const BillboardRenderCommand> GetBillboards()
        const noexcept;
    [[nodiscard]] std::span<const TextRenderCommand> GetText() const noexcept;
    [[nodiscard]] std::span<const DebugLineRenderCommand> GetDebugLines()
        const noexcept;

private:
    std::vector<MeshRenderCommand> opaqueMeshes_;
    std::vector<MeshRenderCommand> transparentMeshes_;
    std::vector<BillboardRenderCommand> billboards_;
    std::vector<TextRenderCommand> text_;
    std::vector<DebugLineRenderCommand> debugLines_;
};

#endif  // RENDER_QUEUE_HPP_INCLUDED
