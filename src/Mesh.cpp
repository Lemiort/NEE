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
#include <utility>
#include <vector>

#include "Camera.hpp"
#include "Material.hpp"
#include "ModelLoader.hpp"
#include "Transform.hpp"
#include "spdlog/spdlog.h"

namespace {
constexpr std::size_t kPositionComponentCount = 3;
constexpr std::size_t kTextureCoordinateComponentCount = 2;
constexpr std::size_t kNormalComponentCount = 3;
constexpr std::size_t kGpuVertexComponentCount =
    kPositionComponentCount + kNormalComponentCount +
    kTextureCoordinateComponentCount + kNormalComponentCount;
constexpr std::size_t kTriangleIndexCount = 3;

const GLvoid* BufferOffset(std::size_t byteOffset) {
    // With a VBO bound, OpenGL interprets this pointer value as a byte offset.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,
    // performance-no-int-to-ptr)
    return reinterpret_cast<const GLvoid*>(
        static_cast<std::uintptr_t>(byteOffset));
}
}  // namespace

Mesh::~Mesh() = default;

MeshShaderBindings MeshShaderBindings::FromShader(const Shader& shader) {
    MeshShaderBindings bindings{
        .position = shader.GetAttribLocation("s_vPosition"),
        .normal = shader.GetAttribLocation("s_vNormal"),
        .uv = shader.GetAttribLocation("s_vUV"),
        .tangent = shader.GetAttribLocation("s_vTangent"),
        .world = shader.GetUniformLocation("gWorld"),
        .rotation = shader.GetUniformLocation("mRotate"),
        .instanceWorld = shader.GetAttribLocation("instanceWorld"),
        .instanceRotation = shader.GetAttribLocation("instanceRotation"),
        .instanced = shader.GetUniformLocation("instanced")};
    if (bindings.position < 0) {
        throw std::invalid_argument(
            "Mesh shader must declare an active s_vPosition attribute");
    }
    return bindings;
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
        vertexCount >
            maxBufferSize / (sizeof(float) * kGpuVertexComponentCount) ||
        indexCount > maxBufferSize / sizeof(std::uint32_t) ||
        indexCount % kTriangleIndexCount != 0) {
        spdlog::error("Model '{}' exceeds supported mesh limits", model);
        throw std::length_error("Mesh model exceeds supported limits: " +
                                model);
    }

    std::vector<float> spvertices;
    std::vector<float> spnormals;
    std::vector<float> spuvs;
    std::vector<float> sptangent;
    try {
        spvertices.reserve(vertexCount * kPositionComponentCount);
        spnormals.reserve(vertexCount * kNormalComponentCount);
        spuvs.reserve(vertexCount * kTextureCoordinateComponentCount);
        sptangent.reserve(vertexCount * kNormalComponentCount);
        for (const ModelVertex& vertex : modelData.vertices) {
            spvertices.insert(spvertices.end(), vertex.position.begin(),
                              vertex.position.end());
            spnormals.insert(spnormals.end(), vertex.normal.begin(),
                             vertex.normal.end());
            spuvs.insert(spuvs.end(), vertex.uv.begin(), vertex.uv.end());
            sptangent.insert(sptangent.end(), vertex.tangent.begin(),
                             vertex.tangent.end());
        }
    } catch (const std::bad_alloc&) {
        spdlog::error("Unable to allocate GPU data for mesh model '{}'", model);
        throw;
    }

    vertexCount_ = static_cast<int>(vertexCount);
    faceCount_ = static_cast<int>(indexCount / kTriangleIndexCount);

    // create buffer to store everything
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    const auto positionBytes =
        static_cast<GLsizeiptr>(sizeof(float) * kPositionComponentCount *
                                static_cast<std::size_t>(vertexCount_));
    const auto normalBytes =
        static_cast<GLsizeiptr>(sizeof(float) * kNormalComponentCount *
                                static_cast<std::size_t>(vertexCount_));
    const auto textureCoordinateBytes = static_cast<GLsizeiptr>(
        sizeof(float) * kTextureCoordinateComponentCount *
        static_cast<std::size_t>(vertexCount_));
    const GLsizeiptr tangentBytes = normalBytes;
    const GLsizeiptr vertexBufferBytes =
        positionBytes + normalBytes + textureCoordinateBytes + tangentBytes;
    const auto indexBufferBytes =
        static_cast<GLsizeiptr>(sizeof(std::uint32_t) * kTriangleIndexCount *
                                static_cast<std::size_t>(faceCount_));
    glBufferData(GL_ARRAY_BUFFER, vertexBufferBytes, nullptr, GL_STATIC_DRAW);
    // load vertices into buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, positionBytes, spvertices.data());
    // normals
    glBufferSubData(GL_ARRAY_BUFFER, positionBytes, normalBytes,
                    spnormals.data());
    // texture coordinates
    glBufferSubData(GL_ARRAY_BUFFER, positionBytes + normalBytes,
                    textureCoordinateBytes, spuvs.data());
    // tangent
    glBufferSubData(GL_ARRAY_BUFFER,
                    positionBytes + normalBytes + textureCoordinateBytes,
                    tangentBytes, sptangent.data());

    // bind indices to buffer
    glGenBuffers(1, &IBO);
    if (VBO == 0 || IBO == 0) {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &IBO);
        VBO = 0;
        IBO = 0;
        spdlog::error("Unable to create GPU buffers for mesh model '{}'",
                      model);
        throw std::runtime_error("Unable to create mesh GPU buffers: " + model);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferBytes,
                 modelData.indices.data(), GL_STATIC_DRAW);

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

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    const auto configureAttribute = [](GLint location, GLint componentCount,
                                       const GLvoid* offset) {
        if (location < 0) return;
        const auto attribute = static_cast<GLuint>(location);
        glVertexAttribPointer(attribute, componentCount, GL_FLOAT, GL_FALSE, 0,
                              offset);
        glEnableVertexAttribArray(attribute);
    };
    configureAttribute(bindings.position, 3, nullptr);
    configureAttribute(bindings.normal, 3,
                       BufferOffset(sizeof(float) * 3 *
                                    static_cast<std::size_t>(vertexCount_)));
    configureAttribute(bindings.uv, 2,
                       BufferOffset(sizeof(float) * 6 *
                                    static_cast<std::size_t>(vertexCount_)));
    configureAttribute(bindings.tangent, 3,
                       BufferOffset(sizeof(float) * 8 *
                                    static_cast<std::size_t>(vertexCount_)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glUniformMatrix4fv(bindings.world, 1, GL_FALSE,
                       glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(bindings.rotation, 1, GL_FALSE,
                       glm::value_ptr(normalMatrix));

    if (instanceBuffer != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
        for (GLuint column = 0; column < 4; ++column) {
            for (GLuint matrix = 0; matrix < 2; ++matrix) {
                const GLuint location =
                    static_cast<GLuint>(matrix == 0
                                            ? bindings.instanceWorld
                                            : bindings.instanceRotation) +
                    column;
                glEnableVertexAttribArray(location);
                glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE,
                                      2 * sizeof(glm::mat4),
                                      BufferOffset(matrix * sizeof(glm::mat4) +
                                                   column * sizeof(glm::vec4)));
                glVertexAttribDivisor(location, 1);
            }
        }
    }
    glDrawElementsInstanced(
        GL_TRIANGLES,
        static_cast<GLsizei>(static_cast<std::size_t>(faceCount_) *
                             kTriangleIndexCount),
        GL_UNSIGNED_INT, nullptr, instanceCount);
    if (instanceBuffer != 0) {
        for (GLuint column = 0; column < 4; ++column) {
            for (const GLint base :
                 {bindings.instanceWorld, bindings.instanceRotation}) {
                const GLuint location = static_cast<GLuint>(base) + column;
                glVertexAttribDivisor(location, 0);
                glDisableVertexAttribArray(location);
            }
        }
    }
    for (const GLint location :
         {bindings.position, bindings.normal, bindings.uv, bindings.tangent}) {
        if (location >= 0)
            glDisableVertexAttribArray(static_cast<GLuint>(location));
    }
}

int Mesh::GetNumFaces() const { return faceCount_; }

int Mesh::GetNumVerts() const { return vertexCount_; }
