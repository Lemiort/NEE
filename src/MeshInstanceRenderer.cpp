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
                                  const Material* passMaterial) {
    static_assert(sizeof(InstanceData) == 2 * sizeof(glm::mat4));
    sorted_.clear();
    for (const auto& command : commands) {
        if (passMaterial != nullptr || command.material != nullptr)
            sorted_.push_back(&command);
    }
    const auto materialFor = [passMaterial](const MeshRenderCommand* command) {
        return passMaterial != nullptr ? passMaterial : command->material.get();
    };
    std::ranges::stable_sort(sorted_, [&](const auto* a, const auto* b) {
        if (&a->mesh.get() != &b->mesh.get())
            return std::less<const Mesh*>{}(&a->mesh.get(), &b->mesh.get());
        return std::less<const Material*>{}(materialFor(a), materialFor(b));
    });
    for (std::size_t begin = 0; begin < sorted_.size();) {
        std::size_t end = begin + 1;
        while (end < sorted_.size() &&
               &sorted_[end]->mesh.get() == &sorted_[begin]->mesh.get() &&
               materialFor(sorted_[end]) == materialFor(sorted_[begin]))
            ++end;
        const auto& first = *sorted_[begin];
        const Material& material = *materialFor(&first);
        if (end - begin == 1 || bindings.instanceWorld < 0 ||
            bindings.instanceRotation < 0 || bindings.instanced < 0) {
            for (std::size_t index = begin; index < end; ++index)
                first.mesh.get().Draw(sorted_[index]->transform, material,
                                      bindings);
        } else {
            instances_.clear();
            for (std::size_t index = begin; index < end; ++index) {
                const auto& transform = sorted_[index]->transform;
                instances_.push_back(
                    {transform.ModelMatrix(), transform.NormalMatrix()});
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
                glGenBuffers(1, &buffer_);
                if (buffer_ == 0)
                    throw std::runtime_error(
                        "Cannot create mesh instance buffer");
            }
            glBindBuffer(GL_ARRAY_BUFFER, buffer_);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(instances_.size() *
                                                 sizeof(InstanceData)),
                         instances_.data(), GL_STREAM_DRAW);
            first.mesh.get().Draw(first.transform, material, bindings, 1.0F,
                                  buffer_,
                                  static_cast<GLsizei>(instances_.size()));
        }
        begin = end;
    }
}
