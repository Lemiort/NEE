#include "Skybox.hpp"

#include <glad/gl.h>

#include <array>
#include <cstdio>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <memory>
#include <new>
#include <string>
#include <vector>
#ifdef _MSC_VER
#include <stdio.h>
#endif

#include "Camera.hpp"
#include "CubemapTexture.hpp"
#include "Shader.hpp"
#include "spdlog/spdlog.h"

namespace {
constexpr std::size_t kPositionComponentCount = 3;
constexpr std::size_t kVertexComponentCount = 8;
constexpr std::size_t kTriangleIndexCount = 3;

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

SkyBox::SkyBox(const shared_ptr<Shader>& shader)
    : shaderProgramID(shader->GetShaderProgramId()) {
  shaderProgram = shader;
}
SkyBox::~SkyBox() = default;
bool SkyBox::Init(const string& Directory, const string& PosXFilename,
                  const string& NegXFilename, const string& PosYFilename,
                  const string& NegYFilename, const string& PosZFilename,
                  const string& NegZFilename) {
    Scale = 0;
    std::vector<int> spindices;
    std::vector<float> spvertices;
    std::vector<float> sptangent;
    spfaces = 0;
    spverts = 0;
    try {
        std::unique_ptr<std::FILE, decltype(&std::fclose)> fp(
        OpenBinaryFile("models/sphere.ho3d"), &std::fclose);
        if (fp == nullptr) {
            spdlog::error("Unable to open skybox model: models/sphere.ho3d");
            return false;
        }
        const auto read = [&fp]<typename T>(T* destination,
                                             std::size_t count) {
            return std::fread(destination, sizeof(T), count, fp.get()) == count;
        };
        int vertexCount = 0;
        if (!read(&vertexCount, 1) || vertexCount <= 0) {
            spdlog::error("Skybox model has an invalid vertex count");
            return false;
        }
        const auto vertexCountSize = static_cast<std::size_t>(vertexCount);
        if (vertexCountSize >
                std::vector<float>{}.max_size() / kPositionComponentCount ||
            vertexCountSize >
                static_cast<std::size_t>(
                    std::numeric_limits<GLsizeiptr>::max()) /
                    (sizeof(float) * kPositionComponentCount)) {
            spdlog::error("Skybox model vertex count is too large");
            return false;
        }
        spvertices.resize(vertexCountSize * kPositionComponentCount);
        sptangent.resize(vertexCountSize * kPositionComponentCount);

        std::array<float, kVertexComponentCount> vertex{};
        for (std::size_t i = 0; i < vertexCountSize; ++i) {
            if (!read(vertex.data(), vertex.size())) {
                spdlog::error("Skybox model has truncated vertex data");
                return false;
            }
            const std::size_t positionOffset = i * kPositionComponentCount;
            spvertices.at(positionOffset) = vertex.at(0);
            spvertices.at(positionOffset + 1) = vertex.at(1);
            spvertices.at(positionOffset + 2) = vertex.at(2);
        }
        if (!read(sptangent.data(), sptangent.size())) {
            spdlog::error("Skybox model has truncated tangent data");
            return false;
        }
        int faceCount = 0;
        if (!read(&faceCount, 1) || faceCount <= 0) {
            spdlog::error("Skybox model has an invalid face count");
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
            spdlog::error("Skybox model face count is too large");
            return false;
        }
        spfaces = faceCount;
        spindices.resize(faceCountSize * kTriangleIndexCount);
        if (!read(spindices.data(), spindices.size())) {
            spdlog::error("Skybox model has truncated index data");
            return false;
        }
        spverts = vertexCount;
    } catch (const std::bad_alloc&) {
        spdlog::error("Error creating make_shared<Mesh> in Skybox");
        return false;
    }
    // create buffer to store everything
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    const auto vertexBufferBytes = static_cast<GLsizeiptr>(
        sizeof(float) * kPositionComponentCount *
        static_cast<std::size_t>(spverts));
    const auto indexBufferBytes = static_cast<GLsizeiptr>(
        sizeof(int) * kTriangleIndexCount * static_cast<std::size_t>(spfaces));
    glBufferData(GL_ARRAY_BUFFER, vertexBufferBytes, nullptr, GL_STATIC_DRAW);
    // load vertices into buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBufferBytes, spvertices.data());

    // bind indices to buffer
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferBytes, spindices.data(),
                 GL_STATIC_DRAW);

    try {
        pCubemapTex = std::make_unique<CubemapTexture>(
            Directory, PosXFilename, NegXFilename, PosYFilename, NegYFilename,
            PosZFilename, NegZFilename);
    } catch (const std::bad_alloc&) {
        spdlog::error("Error creating new Cubemap");
        return false;
    }
    pCubemapTex->Load();

    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    // find uniform variable addresses
    gWorldID = shaderProgram->GetUniformLocation("gWVP");

    // do the same thing
    pCubemapTex->Bind(GL_TEXTURE2);
    textureID = shaderProgram->GetUniformLocation("gCubemapTexture");
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(textureID, 2);
    position.fill(0.0F);
    rotation.fill(0.0F);
    scale.fill(1.0F);

    // clean up memory
    // vectors automatically cleaned up
    return true;
}
void SkyBox::Render(const Camera& cam) {
    GLint OldCullFaceMode = 0;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    GLint OldDepthFuncMode = 0;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_FRONT);
    glDepthFunc(GL_LEQUAL);

    const glm::mat4 modelMatrix =
        glm::translate(glm::mat4(1.0F), cam.GetPos()) *
        glm::scale(glm::mat4(1.0F), glm::vec3(3.0F));
    const glm::mat4 worldViewProjection =
        cam.GetViewProjectionMatrix() * modelMatrix;

    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    pCubemapTex->Bind(GL_TEXTURE2);
    glUniform1i(textureID, 2);

    glUniformMatrix4fv(gWorldID, 1, GL_FALSE,
                       glm::value_ptr(worldViewProjection));

    glEnableVertexAttribArray(positionID);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(
                       static_cast<std::size_t>(spfaces) *
                       kTriangleIndexCount),
                   GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(positionID);

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
}
