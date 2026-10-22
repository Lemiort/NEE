#include "RenderQueue.hpp"

#include <utility>

void RenderQueue::SubmitOpaqueMesh(Mesh& mesh, const Transform& transform) {
    opaqueMeshes_.push_back({.mesh = mesh, .transform = transform});
}

void RenderQueue::SubmitTransparentMesh(Mesh& mesh, const Transform& transform,
                                        float opacity) {
    transparentMeshes_.push_back({.mesh = mesh,
                                  .transform = transform,
                                  .layer = RenderLayer::Transparent,
                                  .opacity = opacity});
}

void RenderQueue::SubmitBillboard(Billboard& billboard,
                                  const glm::vec3& position) {
    billboards_.push_back({.billboard = billboard, .position = position});
}

void RenderQueue::SubmitText(FontLine2d text) {
    text_.push_back({.text = std::move(text)});
}

void RenderQueue::SubmitDebugLine(const DebugLineRenderCommand& line) {
    debugLines_.push_back(line);
}

void RenderQueue::Clear() noexcept {
    opaqueMeshes_.clear();
    transparentMeshes_.clear();
    billboards_.clear();
    text_.clear();
    debugLines_.clear();
}

void RenderQueue::ClearText() noexcept { text_.clear(); }

std::span<const MeshRenderCommand> RenderQueue::GetOpaqueMeshes()
    const noexcept {
    return opaqueMeshes_;
}

std::span<const MeshRenderCommand> RenderQueue::GetTransparentMeshes()
    const noexcept {
    return transparentMeshes_;
}

std::span<const BillboardRenderCommand> RenderQueue::GetBillboards()
    const noexcept {
    return billboards_;
}

std::span<const TextRenderCommand> RenderQueue::GetText() const noexcept {
    return text_;
}

std::span<const DebugLineRenderCommand> RenderQueue::GetDebugLines()
    const noexcept {
    return debugLines_;
}
