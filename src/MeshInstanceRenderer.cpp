#include "MeshInstanceRenderer.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <stdexcept>

MeshInstanceRenderer::~MeshInstanceRenderer() {
    if (buffer_ != 0) glDeleteBuffers(1, &buffer_);
}

void MeshInstanceRenderer::Render(std::span<const MeshRenderCommand> commands,
                                  const MeshShaderBindings& bindings,
                                  Shader* passShader, const Frustum* frustum,
                                  RenderStatistics* statistics) {
    static_assert(sizeof(InstanceData) == 2 * sizeof(glm::mat4));
    RenderStatistics counts;
    entries_.clear();
    for (const auto& command : commands) {
        if (command.material == nullptr) continue;
        const glm::mat4 world = command.transform.ModelMatrix();
        if (frustum != nullptr &&
            !frustum->Intersects(
                command.mesh.get().GetBounds().Transformed(world))) {
            ++counts.meshesCulled;
            continue;
        }
        entries_.push_back({&command, world});
    }
    counts.meshesDrawn = entries_.size();
    std::ranges::stable_sort(entries_, [&](const Entry& a, const Entry& b) {
        const auto* meshA = &a.command->mesh.get();
        const auto* meshB = &b.command->mesh.get();
        if (meshA != meshB) return std::less<const Mesh*>{}(meshA, meshB);
        return std::less<const Material*>{}(a.command->material.get(),
                                            b.command->material.get());
    });
    for (std::size_t begin = 0; begin < entries_.size();) {
        std::size_t end = begin + 1;
        const auto& first = *entries_[begin].command;
        while (end < entries_.size() &&
               &entries_[end].command->mesh.get() == &first.mesh.get() &&
               entries_[end].command->material == first.material)
            ++end;
        const Material& material = *first.material;
        if (end - begin == 1 || bindings.instanceWorld < 0 ||
            bindings.instanceRotation < 0 || bindings.instanced < 0) {
            for (std::size_t index = begin; index < end; ++index) {
                first.mesh.get().Draw(entries_[index].command->transform,
                                      material, bindings, 1.0F, 0, 1,
                                      passShader);
                ++counts.drawCalls;
            }
        } else {
            instances_.clear();
            for (std::size_t index = begin; index < end; ++index) {
                instances_.push_back(
                    {entries_[index].world,
                     entries_[index].command->transform.NormalMatrix()});
            }
            if (instances_.size() > static_cast<std::size_t>(
                                        std::numeric_limits<GLsizei>::max()) ||
                instances_.size() >
                    static_cast<std::size_t>(
                        std::numeric_limits<GLsizeiptr>::max()) /
                        sizeof(InstanceData))
                throw std::length_error(
                    "Mesh instance batch exceeds GPU limits");
            if (buffer_ == 0) {
                glCreateBuffers(1, &buffer_);
                if (buffer_ == 0)
                    throw std::runtime_error(
                        "Cannot create mesh instance buffer");
            }
            glNamedBufferData(buffer_,
                              static_cast<GLsizeiptr>(instances_.size() *
                                                      sizeof(InstanceData)),
                              instances_.data(), GL_STREAM_DRAW);
            first.mesh.get().Draw(
                first.transform, material, bindings, 1.0F, buffer_,
                static_cast<GLsizei>(instances_.size()), passShader);
            ++counts.drawCalls;
        }
        begin = end;
    }
    if (statistics != nullptr) {
        statistics->meshesDrawn += counts.meshesDrawn;
        statistics->meshesCulled += counts.meshesCulled;
        statistics->drawCalls += counts.drawCalls;
    }
}

void MeshInstanceRenderer::RenderBatches(
    std::span<const std::reference_wrapper<const MeshBatch>> batches,
    const MeshShaderBindings& bindings, Shader* passShader,
    const Frustum* frustum, RenderStatistics* statistics) {
    RenderStatistics counts;
    for (const MeshBatch& batch : batches) {
        if (batch.GetCount() == 0) continue;
        if (frustum != nullptr && !frustum->Intersects(batch.GetBounds())) {
            ++counts.batchesCulled;
            continue;
        }
        batch.Draw(bindings, passShader);
        ++counts.batchesDrawn;
        counts.batchInstancesDrawn += batch.GetCount();
        ++counts.drawCalls;
    }
    if (statistics != nullptr) {
        statistics->batchesDrawn += counts.batchesDrawn;
        statistics->batchesCulled += counts.batchesCulled;
        statistics->batchInstancesDrawn += counts.batchInstancesDrawn;
        statistics->drawCalls += counts.drawCalls;
    }
}
