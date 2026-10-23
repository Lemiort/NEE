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
    return {.position = shader.GetAttribLocation("s_vPosition"),
            .normal = shader.GetAttribLocation("s_vNormal"),
            .uv = shader.GetAttribLocation("s_vUV"),
            .tangent = shader.GetAttribLocation("s_vTangent"),
            .world = shader.GetUniformLocation("gWorld"),
            .rotation = shader.GetUniformLocation("mRotate")};
}

void Mesh::SetMaterial(shared_ptr<Material> _mat) {
    mat = _mat;
    this->SetShader(mat->GetShader());

    // mat->Use();
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    normalID = shaderProgram->GetAttribLocation("s_vNormal");
    uvID = shaderProgram->GetAttribLocation("s_vUV");
    tangentID = shaderProgram->GetAttribLocation("s_vTangent");
    // find uniform variable positions
    gWorldID = shaderProgram->GetUniformLocation("gWorld");
    rotateID =
        shaderProgram->GetUniformLocation("mRotate");  // rotation of object
}

Mesh::Mesh(gsl_lite::not_null<shared_ptr<Material>> material,
           const std::string& model) {
    mat = std::move(material);
    if (model.empty()) {
        spdlog::error("Cannot construct mesh without a model");
        throw std::invalid_argument("Mesh model path must not be empty");
    }
    shaderProgram = mat->GetShader();

    Scale = 0;
    spfaces = 0;
    spverts = 0;
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

    spverts = static_cast<int>(vertexCount);
    spfaces = static_cast<int>(indexCount / kTriangleIndexCount);

    // create buffer to store everything
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    const auto positionBytes =
        static_cast<GLsizeiptr>(sizeof(float) * kPositionComponentCount *
                                static_cast<std::size_t>(spverts));
    const auto normalBytes =
        static_cast<GLsizeiptr>(sizeof(float) * kNormalComponentCount *
                                static_cast<std::size_t>(spverts));
    const auto textureCoordinateBytes = static_cast<GLsizeiptr>(
        sizeof(float) * kTextureCoordinateComponentCount *
        static_cast<std::size_t>(spverts));
    const GLsizeiptr tangentBytes = normalBytes;
    const GLsizeiptr vertexBufferBytes =
        positionBytes + normalBytes + textureCoordinateBytes + tangentBytes;
    const auto indexBufferBytes =
        static_cast<GLsizeiptr>(sizeof(std::uint32_t) * kTriangleIndexCount *
                                static_cast<std::size_t>(spfaces));
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

    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    normalID = shaderProgram->GetAttribLocation("s_vNormal");
    uvID = shaderProgram->GetAttribLocation("s_vUV");
    tangentID = shaderProgram->GetAttribLocation("s_vTangent");

    // find uniform variable positions
    gWorldID = shaderProgram->GetUniformLocation("gWorld");
    rotateID =
        shaderProgram->GetUniformLocation("mRotate");  // rotation of object

    transform_ = Transform{};
}
void Mesh::Render(const Camera& /*cam*/) {
    Draw(transform_, *mat, MeshShaderBindings::FromShader(*mat->GetShader()));
}

void Mesh::Draw(const Transform& transform, const Material& material,
                const MeshShaderBindings& bindings, float opacity) const {
    const glm::mat4 modelMatrix = transform.ModelMatrix();
    const glm::mat4 rotationMatrix = transform.RotationMatrix();

    material.Use(opacity);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(bindings.position, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(
        bindings.normal, 3, GL_FLOAT, GL_FALSE, 0,
        BufferOffset(sizeof(float) * 3 * static_cast<std::size_t>(spverts)));
    glVertexAttribPointer(
        bindings.uv, 2, GL_FLOAT, GL_FALSE, 0,
        BufferOffset(sizeof(float) * 6 * static_cast<std::size_t>(spverts)));
    glVertexAttribPointer(
        bindings.tangent, 3, GL_FLOAT, GL_FALSE, 0,
        BufferOffset(sizeof(float) * 8 * static_cast<std::size_t>(spverts)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glUniformMatrix4fv(bindings.world, 1, GL_FALSE,
                       glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(bindings.rotation, 1, GL_FALSE,
                       glm::value_ptr(rotationMatrix));  // rotation of model

    glEnableVertexAttribArray(bindings.position);
    glEnableVertexAttribArray(bindings.normal);
    glEnableVertexAttribArray(bindings.uv);
    glEnableVertexAttribArray(bindings.tangent);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(static_cast<std::size_t>(spfaces) *
                                        kTriangleIndexCount),
                   GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(bindings.position);
    glDisableVertexAttribArray(bindings.normal);
    glDisableVertexAttribArray(bindings.uv);
    glDisableVertexAttribArray(bindings.tangent);
}

int Mesh::GetNumFaces() const { return spfaces; }

int Mesh::GetNumVerts() const { return spverts; }

void Mesh::SetTexture(GLuint textureUnit) { shadowMap = textureUnit; }

/*shared_ptr<Shader> Mesh::GetShader()
{
    return shaderProgram;
}
void Mesh::SetShader(shared_ptr<Shader> shader)
{
    shaderProgram = shader;
}*/
