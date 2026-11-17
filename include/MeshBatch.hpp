#ifndef MESH_BATCH_HPP_INCLUDED
#define MESH_BATCH_HPP_INCLUDED

#include <glad/gl.h>

#include <cstddef>
#include <functional>
#include <gsl-lite/gsl-lite.hpp>
#include <memory>
#include <span>

#include "Bounds.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"

// Many copies of one mesh in one material that never move, such as the
// windows of a building: their instance data is computed and uploaded once,
// and each frame they draw with a single instanced call. Queue it with
// RenderQueue::SubmitOpaqueBatch. The mesh is borrowed and must outlive the
// batch. Must be created and destroyed while the OpenGL context is current.
class MeshBatch {
public:
    MeshBatch(const Mesh& mesh,
              gsl_lite::not_null<std::shared_ptr<const Material>> material,
              std::span<const Transform> transforms);
    ~MeshBatch();
    MeshBatch(const MeshBatch&) = delete;
    MeshBatch& operator=(const MeshBatch&) = delete;
    MeshBatch(MeshBatch&& other) noexcept;
    MeshBatch& operator=(MeshBatch&& other) noexcept;

    [[nodiscard]] const Mesh& GetMesh() const noexcept { return mesh_.get(); }
    [[nodiscard]] const Material& GetMaterial() const noexcept {
        return *material_;
    }
    [[nodiscard]] std::size_t GetCount() const noexcept { return count_; }
    // The box around every instance, in world space.
    [[nodiscard]] const Bounds& GetBounds() const noexcept { return bounds_; }

    // Draws every instance. A pass shader overrides the material's shader
    // while keeping its textures, as MeshInstanceRenderer does.
    void Draw(const MeshShaderBindings& bindings,
              Shader* passShader = nullptr) const;

private:
    std::reference_wrapper<const Mesh> mesh_;
    gsl_lite::not_null<std::shared_ptr<const Material>> material_;
    GLuint buffer_{};
    std::size_t count_{};
    Bounds bounds_;
};

#endif  // MESH_BATCH_HPP_INCLUDED
