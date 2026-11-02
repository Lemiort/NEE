#include "Mesh.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "Camera.hpp"
#include "Material.hpp"
#include "ModelLoader.hpp"
#include "Transform.hpp"
#include "spdlog/spdlog.h"

namespace {
constexpr std::size_t kTriangleIndexCount = 3;
}  // namespace

Mesh::~Mesh() = default;

MeshShaderBindings MeshShaderBindings::FromShader(const Shader& shader) {
    return {.position = shader.GetAttribLocation("s_vPosition"),
            .normal = shader.GetAttribLocation("s_vNormal"),
            .uv = shader.GetAttribLocation("s_vUV"),
            .tangent = shader.GetAttribLocation("s_vTangent"),
            .world = shader.GetUniformLocation("gWorld"),
            .rotation = shader.GetUniformLocation("mRotate"),
            .instanceWorld = shader.GetAttribLocation("instanceWorld"),
            .instanceRotation = shader.GetAttribLocation("instanceRotation"),
            .instanced = shader.GetUniformLocation("instanced")};
}

void Mesh::SetMaterial(std::shared_ptr<Material> material) {
    mat = std::move(material);
    this->SetShader(mat->GetShader());
}

Mesh::Mesh(gsl_lite::not_null<std::shared_ptr<Material>> material,
           const std::string& model) {
    mat = std::move(material);
    if (model.empty()) {
        spdlog::error("Cannot construct mesh without a model");
        throw std::invalid_argument("Mesh model path must not be empty");
    }
    shaderProgram = mat->GetShader();

    ModelData modelData;
    try {
        if (!LoadModel(model, modelData)) {
            throw std::runtime_error("Unable to load mesh model: " + model);
        }
    } catch (const std::bad_alloc&) {
        spdlog::error("Unable to allocate data for mesh model '{}'", model);
        throw;
    }

    const std::size_t vertexCount = modelData.vertices.size();
    const std::size_t indexCount = modelData.indices.size();
    const auto maxBufferSize =
        static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max());
    if (vertexCount >
            static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
        indexCount >
            static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()) ||
        vertexCount > maxBufferSize / sizeof(ModelVertex) ||
        indexCount > maxBufferSize / sizeof(std::uint32_t) ||
        indexCount % kTriangleIndexCount != 0) {
        spdlog::error("Model '{}' exceeds supported mesh limits", model);
        throw std::length_error("Mesh model exceeds supported limits: " +
                                model);
    }

    vertexCount_ = static_cast<int>(vertexCount);
    faceCount_ = static_cast<int>(indexCount / kTriangleIndexCount);

    static_assert(std::is_standard_layout_v<ModelVertex>);
    // Upload the loader's interleaved vertices directly into immutable storage.
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &IBO);
    if (VBO == 0 || IBO == 0) {
        throw std::runtime_error("Unable to create mesh GPU buffers: " + model);
    }
    glNamedBufferStorage(
        VBO, static_cast<GLsizeiptr>(vertexCount * sizeof(ModelVertex)),
        modelData.vertices.data(), 0);
    glNamedBufferStorage(
        IBO, static_cast<GLsizeiptr>(indexCount * sizeof(std::uint32_t)),
        modelData.indices.data(), 0);

    transform_ = Transform{};
}
void Mesh::Render(const Camera& /*cam*/) {
    Draw(transform_, *mat, MeshShaderBindings::FromShader(*mat->GetShader()));
}

void Mesh::Draw(const Transform& transform, const Material& material,
                const MeshShaderBindings& bindings, float opacity,
                GLuint instanceBuffer, GLsizei instanceCount,
                Shader* passShader) const {
    const glm::mat4 modelMatrix = transform.ModelMatrix();
    const glm::mat4 normalMatrix = transform.NormalMatrix();

    if (passShader != nullptr)
        material.UseWithShader(*passShader, opacity);
    else
        material.Use(opacity);
    glUniform1i(bindings.instanced, instanceBuffer != 0);

    const std::array<GLint, 6> layout{
        bindings.position,
        bindings.normal,
        bindings.uv,
        bindings.tangent,
        instanceBuffer != 0 ? bindings.instanceWorld : -1,
        instanceBuffer != 0 ? bindings.instanceRotation : -1};
    auto [entry, created] = vertexArrays_.try_emplace(layout);
    auto& vao = entry->second;
    if (created) {
        vao.SetBuffer(0, VBO, sizeof(ModelVertex));
        vao.SetIndices(IBO);
        vao.SetAttribute(bindings.position, 0, 3,
                         offsetof(ModelVertex, position));
        vao.SetAttribute(bindings.normal, 0, 3, offsetof(ModelVertex, normal));
        vao.SetAttribute(bindings.uv, 0, 2, offsetof(ModelVertex, uv));
        vao.SetAttribute(bindings.tangent, 0, 3,
                         offsetof(ModelVertex, tangent));
        if (instanceBuffer != 0) {
            for (GLint column = 0; column < 4; ++column) {
                vao.SetAttribute(bindings.instanceWorld + column, 1, 4,
                                 column * sizeof(glm::vec4));
                vao.SetAttribute(
                    bindings.instanceRotation + column, 1, 4,
                    sizeof(glm::mat4) + column * sizeof(glm::vec4));
            }
            vao.SetDivisor(1, 1);
        }
    }
    if (instanceBuffer != 0)
        vao.SetBuffer(1, instanceBuffer, 2 * sizeof(glm::mat4));
    vao.Bind();

    glUniformMatrix4fv(bindings.world, 1, GL_FALSE,
                       glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(bindings.rotation, 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));

    glDrawElementsInstanced(
        GL_TRIANGLES,
        static_cast<GLsizei>(static_cast<std::size_t>(faceCount_) *
                             kTriangleIndexCount),
        GL_UNSIGNED_INT, nullptr, instanceCount);
}

int Mesh::GetNumFaces() const { return faceCount_; }

int Mesh::GetNumVerts() const { return vertexCount_; }
