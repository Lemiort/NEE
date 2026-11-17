#include "MeshBatch.hpp"

#include <glad/gl.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
// The layout Mesh::Draw reads instance data in: world matrix, then normal
// matrix.
struct InstanceData {
    glm::mat4 world;
    glm::mat4 normal;
};
}  // namespace

MeshBatch::MeshBatch(const Mesh& mesh, std::shared_ptr<const Material> material,
                     std::span<const Transform> transforms)
    : mesh_(mesh), material_(std::move(material)), count_(transforms.size()) {
    if (material_ == nullptr) {
        throw std::invalid_argument("A mesh batch needs a material");
    }
    if (count_ == 0) {
        return;
    }
    if (count_ >
            static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()) ||
        count_ >
            static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) /
                sizeof(InstanceData)) {
        throw std::length_error("Mesh batch exceeds GPU limits");
    }
    std::vector<InstanceData> instances;
    instances.reserve(count_);
    for (const Transform& transform : transforms) {
        const glm::mat4 world = transform.ModelMatrix();
        instances.push_back({world, transform.NormalMatrix()});
        bounds_.Grow(mesh.GetBounds().Transformed(world));
    }
    glCreateBuffers(1, &buffer_);
    if (buffer_ == 0) {
        throw std::runtime_error("Cannot create mesh batch buffer");
    }
    glNamedBufferStorage(
        buffer_,
        static_cast<GLsizeiptr>(instances.size() * sizeof(InstanceData)),
        instances.data(), 0);
}

MeshBatch::~MeshBatch() {
    if (buffer_ != 0) {
        glDeleteBuffers(1, &buffer_);
    }
}

MeshBatch::MeshBatch(MeshBatch&& other) noexcept
    : mesh_(other.mesh_),
      material_(std::move(other.material_)),
      buffer_(std::exchange(other.buffer_, 0)),
      count_(std::exchange(other.count_, 0)),
      bounds_(std::exchange(other.bounds_, Bounds{})) {}

MeshBatch& MeshBatch::operator=(MeshBatch&& other) noexcept {
    if (this != &other) {
        if (buffer_ != 0) {
            glDeleteBuffers(1, &buffer_);
        }
        mesh_ = other.mesh_;
        material_ = std::move(other.material_);
        buffer_ = std::exchange(other.buffer_, 0);
        count_ = std::exchange(other.count_, 0);
        bounds_ = std::exchange(other.bounds_, Bounds{});
    }
    return *this;
}

void MeshBatch::Draw(const MeshShaderBindings& bindings,
                     Shader* passShader) const {
    if (count_ == 0 || bindings.instanceWorld < 0 ||
        bindings.instanceRotation < 0 || bindings.instanced < 0) {
        return;
    }
    mesh_.get().Draw(Transform{}, *material_, bindings, 1.0F, buffer_,
                     static_cast<GLsizei>(count_), passShader);
}
