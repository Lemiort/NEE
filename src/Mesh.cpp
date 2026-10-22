#include "Mesh.hpp"

#include <glad/gl.h>

#include <cstdio>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ios>
#include <limits>
#include <memory>
#include <new>
#include <ostream>
#include <utility>
#include <vector>
#ifdef _MSC_VER
#include <stdio.h>
#endif

#include "Camera.hpp"
#include "Material.hpp"
#include "Transform.hpp"
#include "spdlog/spdlog.h"

namespace {
constexpr std::size_t kPositionComponentCount = 3;
constexpr std::size_t kTextureCoordinateComponentCount = 2;
constexpr std::size_t kNormalComponentCount = 3;
constexpr std::size_t kVertexComponentCount =
    kPositionComponentCount + kTextureCoordinateComponentCount +
    kNormalComponentCount;
constexpr std::size_t kGpuVertexComponentCount =
    kPositionComponentCount + kNormalComponentCount +
    kTextureCoordinateComponentCount + kNormalComponentCount;
constexpr std::size_t kTriangleIndexCount = 3;

const GLvoid* BufferOffset(std::size_t byteOffset) {
    // With a VBO bound, OpenGL interprets this pointer value as a byte offset.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
    return reinterpret_cast<const GLvoid*>(
        static_cast<std::uintptr_t>(byteOffset));
}

std::FILE* OpenBinaryFile(const char* path) {
#ifdef _MSC_VER
    std::FILE* file = nullptr;
    if (fopen_s(&file, path, "rb") != 0) {
        return nullptr;
    }
    return file;
#else
    return std::fopen(path, "rb");
#endif
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
    shaderProgram = mat->GetShader();
    // mat->Use();

    if (shaderProgram == nullptr) {
        return false;
    }

    Scale = 0;
    std::vector<int> spindices;
    std::vector<float> spvertices;
    std::vector<float> spnormals;
    std::vector<float> spuvs;
    std::vector<float> sptangent;
    spfaces = 0;
    spverts = 0;
    try {
        std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(
        OpenBinaryFile(model), &std::fclose);
        fstream fout("ImporObj.log", ios::out);
        if (fp == nullptr) {
            spdlog::error("Unable to open mesh model: {}", model);
            return false;
        }
        const auto read = [&fp]<typename T>(T* destination,
                                             std::size_t count) {
            return std::fread(destination, sizeof(T), count, fp.get()) == count;
        };
        int vertexCount = 0;
        if (!read(&vertexCount, 1) || vertexCount <= 0) {
            spdlog::error("Mesh model has an invalid vertex count: {}", model);
            return false;
        }
        const auto vertexCountSize = static_cast<std::size_t>(vertexCount);
        if (vertexCountSize >
                std::vector<float>{}.max_size() / kPositionComponentCount ||
            vertexCountSize >
                static_cast<std::size_t>(
                    std::numeric_limits<GLsizeiptr>::max()) /
                    (sizeof(float) * kGpuVertexComponentCount)) {
            spdlog::error("Mesh model vertex count is too large: {}", model);
            return false;
        }
        spvertices.resize(vertexCountSize * kPositionComponentCount);
        spuvs.resize(vertexCountSize * kTextureCoordinateComponentCount);
        spnormals.resize(vertexCountSize * kNormalComponentCount);
        sptangent.resize(vertexCountSize * kNormalComponentCount);

        std::array<float, kVertexComponentCount> vertex{};
        for (std::size_t i = 0; i < vertexCountSize; ++i) {
            if (!read(vertex.data(), vertex.size())) {
                spdlog::error("Mesh model has truncated vertex data: {}", model);
                return false;
            }
            const std::size_t positionOffset = i * kPositionComponentCount;
            const std::size_t textureOffset =
                i * kTextureCoordinateComponentCount;
            const std::size_t normalOffset = i * kNormalComponentCount;
            spvertices.at(positionOffset) = vertex.at(0);
            spvertices.at(positionOffset + 1) = vertex.at(1);
            spvertices.at(positionOffset + 2) = vertex.at(2);
            spuvs.at(textureOffset) = vertex.at(3);
            spuvs.at(textureOffset + 1) = vertex.at(4);
            spnormals.at(normalOffset) = vertex.at(5);
            spnormals.at(normalOffset + 1) = vertex.at(6);
            spnormals.at(normalOffset + 2) = vertex.at(7);
        }
        if (!read(sptangent.data(), sptangent.size())) {
            spdlog::error("Mesh model has truncated tangent data: {}", model);
            return false;
        }
        spverts = vertexCount;
        fout << spverts << '\n';
        for (std::size_t i = 0; i < vertexCountSize; ++i) {
            const std::size_t positionOffset = i * kPositionComponentCount;
            const std::size_t textureOffset =
                i * kTextureCoordinateComponentCount;
            const std::size_t normalOffset = i * kNormalComponentCount;
            fout << i << " ";
            fout << spvertices.at(positionOffset) << ",";
            fout << spvertices.at(positionOffset + 1) << ",";
            fout << spvertices.at(positionOffset + 2) << ",";
            fout << spuvs.at(textureOffset) << ",";
            fout << spuvs.at(textureOffset + 1) << ",";
            fout << spnormals.at(normalOffset) << ",";
            fout << spnormals.at(normalOffset + 1) << ",";
            fout << spnormals.at(normalOffset + 2) << ",\n";
        }
        int faceCount = 0;
        if (!read(&faceCount, 1) || faceCount <= 0) {
            spdlog::error("Mesh model has an invalid face count: {}", model);
            return false;
        }
        const auto faceCountSize = static_cast<std::size_t>(faceCount);
        if (faceCountSize >
                std::vector<int>{}.max_size() / kTriangleIndexCount ||
            faceCountSize >
                static_cast<std::size_t>(
                    std::numeric_limits<GLsizeiptr>::max()) /
                    (sizeof(int) * kTriangleIndexCount) ||
            faceCountSize >
                static_cast<std::size_t>(
                    std::numeric_limits<GLsizei>::max()) /
                    kTriangleIndexCount) {
            spdlog::error("Mesh model face count is too large: {}", model);
            return false;
        }
        spfaces = faceCount;
        fout << "\n" << spfaces << '\n';
        spindices.resize(faceCountSize * kTriangleIndexCount);
        if (!read(spindices.data(), spindices.size())) {
            spdlog::error("Mesh model has truncated index data: {}", model);
            return false;
        }
        for (std::size_t i = 0; i < faceCountSize; ++i) {
            const std::size_t indexOffset = i * kTriangleIndexCount;
            fout << spindices.at(indexOffset) << ",";
            fout << spindices.at(indexOffset + 1) << ","
                 << spindices.at(indexOffset + 2) << '\n';
        }
    } catch (const std::bad_alloc&) {
        spdlog::error("Error creating make_shared<Mesh>");
        return false;
    }

    // create buffer to store everything
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    const auto positionBytes = static_cast<GLsizeiptr>(
        sizeof(float) * kPositionComponentCount *
        static_cast<std::size_t>(spverts));
    const auto normalBytes = static_cast<GLsizeiptr>(
        sizeof(float) * kNormalComponentCount *
        static_cast<std::size_t>(spverts));
    const auto textureCoordinateBytes = static_cast<GLsizeiptr>(
        sizeof(float) * kTextureCoordinateComponentCount *
        static_cast<std::size_t>(spverts));
    const GLsizeiptr tangentBytes = normalBytes;
    const GLsizeiptr vertexBufferBytes =
        positionBytes + normalBytes + textureCoordinateBytes + tangentBytes;
    const auto indexBufferBytes = static_cast<GLsizeiptr>(
        sizeof(int) * kTriangleIndexCount * static_cast<std::size_t>(spfaces));
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferBytes, spindices.data(),
                 GL_STATIC_DRAW);

    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    normalID = shaderProgram->GetAttribLocation("s_vNormal");
    uvID = shaderProgram->GetAttribLocation("s_vUV");
    tangentID = shaderProgram->GetAttribLocation("s_vTangent");

    // find uniform variable positions
    gWorldID = shaderProgram->GetUniformLocation("gWorld");
    rotateID =
        shaderProgram->GetUniformLocation("mRotate");  // rotation of object

    position.fill(0.0F);
    rotation.fill(0.0F);
    scale.fill(1.0F);

    return true;
}
void Mesh::Render(const Camera& /*cam*/) {
  const Transform transform{
      .position = glm::vec3(position.at(0), position.at(1), position.at(2)),
      .rotationDegrees =
          glm::vec3(rotation.at(0), rotation.at(1), rotation.at(2)),
      .scale = glm::vec3(scale.at(0), scale.at(1), scale.at(2))};
  const glm::mat4 modelMatrix = transform.ModelMatrix();
  const glm::mat4 rotationMatrix = transform.RotationMatrix();

  mat->Use();

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glVertexAttribPointer(normalID, 3, GL_FLOAT, GL_FALSE, 0,
                        BufferOffset(sizeof(float) * 3 *
                                     static_cast<std::size_t>(spverts)));
  glVertexAttribPointer(uvID, 2, GL_FLOAT, GL_FALSE, 0,
                        BufferOffset(sizeof(float) * 6 *
                                     static_cast<std::size_t>(spverts)));
  glVertexAttribPointer(tangentID, 3, GL_FLOAT, GL_FALSE, 0,
                        BufferOffset(sizeof(float) * 8 *
                                     static_cast<std::size_t>(spverts)));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

  glUniformMatrix4fv(gWorldID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
  glUniformMatrix4fv(rotateID, 1, GL_FALSE,
                     glm::value_ptr(rotationMatrix));  // rotation of model

  glEnableVertexAttribArray(positionID);
  glEnableVertexAttribArray(normalID);
  glEnableVertexAttribArray(uvID);
  glEnableVertexAttribArray(tangentID);
  glDrawElements(GL_TRIANGLES,
                  static_cast<GLsizei>(
                      static_cast<std::size_t>(spfaces) *
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
