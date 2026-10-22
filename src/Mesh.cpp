#include "Mesh.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <memory>
#include <new>
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

Mesh::Mesh() {
    VBO = 0;
    IBO = 0;
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
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

bool Mesh::Init(shared_ptr<Material> _mat, const char* model) {
    mat = std::move(_mat);
    if (mat == nullptr || model == nullptr) {
        spdlog::error("Cannot initialize mesh without a material and model");
        return false;
    }
    shaderProgram = mat->GetShader();

    if (shaderProgram == nullptr) {
        return false;
    }

    Scale = 0;
    spfaces = 0;
    spverts = 0;
    ModelData modelData;
    try {
        if (!LoadModel(model, modelData)) {
            return false;
        }
    } catch (const std::bad_alloc&) {
        spdlog::error("Unable to allocate data for mesh model '{}'", model);
        return false;
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
        return false;
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
        return false;
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

    return true;
}
void Mesh::Render(const Camera& /*cam*/) {
    const glm::mat4 modelMatrix = transform_.ModelMatrix();
    const glm::mat4 rotationMatrix = transform_.RotationMatrix();

    mat->Use();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(
        normalID, 3, GL_FLOAT, GL_FALSE, 0,
        BufferOffset(sizeof(float) * 3 * static_cast<std::size_t>(spverts)));
    glVertexAttribPointer(
        uvID, 2, GL_FLOAT, GL_FALSE, 0,
        BufferOffset(sizeof(float) * 6 * static_cast<std::size_t>(spverts)));
    glVertexAttribPointer(
        tangentID, 3, GL_FLOAT, GL_FALSE, 0,
        BufferOffset(sizeof(float) * 8 * static_cast<std::size_t>(spverts)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glUniformMatrix4fv(gWorldID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(rotateID, 1, GL_FALSE,
                       glm::value_ptr(rotationMatrix));  // rotation of model

    glEnableVertexAttribArray(positionID);
    glEnableVertexAttribArray(normalID);
    glEnableVertexAttribArray(uvID);
    glEnableVertexAttribArray(tangentID);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(static_cast<std::size_t>(spfaces) *
                                        kTriangleIndexCount),
                   GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(positionID);
    glDisableVertexAttribArray(normalID);
    glDisableVertexAttribArray(uvID);
    glDisableVertexAttribArray(tangentID);
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
