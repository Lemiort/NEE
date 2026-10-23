#include "Skybox.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <memory>
#include <new>
#include <string>
#include <vector>

#include "Camera.hpp"
#include "CubemapTexture.hpp"
#include "ModelLoader.hpp"
#include "Shader.hpp"
#include "spdlog/spdlog.h"

namespace {
constexpr std::size_t kPositionComponentCount = 3;
constexpr std::size_t kTriangleIndexCount = 3;
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
    ModelData modelData;
    std::vector<float> spvertices;
    spfaces = 0;
    spverts = 0;
    try {
        if (!LoadModel("models/sphere.obj", modelData)) {
            return false;
        }
        const std::size_t vertexCount = modelData.vertices.size();
        const std::size_t indexCount = modelData.indices.size();
        const auto maxBufferSize =
            static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max());
        if (vertexCount >
                static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
            vertexCount >
                maxBufferSize / (sizeof(float) * kPositionComponentCount) ||
            indexCount >
                static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()) ||
            indexCount > maxBufferSize / sizeof(std::uint32_t) ||
            indexCount % kTriangleIndexCount != 0) {
            spdlog::error("Skybox model exceeds supported mesh limits");
            return false;
        }

        spvertices.reserve(vertexCount * kPositionComponentCount);
        for (const ModelVertex& vertex : modelData.vertices) {
            spvertices.insert(spvertices.end(), vertex.position.begin(),
                              vertex.position.end());
        }
        spverts = static_cast<int>(vertexCount);
        spfaces = static_cast<int>(indexCount / kTriangleIndexCount);
    } catch (const std::bad_alloc&) {
        spdlog::error("Unable to allocate data for skybox model");
        return false;
    }
    // create buffer to store everything
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    const auto vertexBufferBytes = static_cast<GLsizeiptr>(
        sizeof(float) * kPositionComponentCount *
        static_cast<std::size_t>(spverts));
    const auto indexBufferBytes = static_cast<GLsizeiptr>(
        sizeof(std::uint32_t) * kTriangleIndexCount *
        static_cast<std::size_t>(spfaces));
    glBufferData(GL_ARRAY_BUFFER, vertexBufferBytes, nullptr, GL_STATIC_DRAW);
    // load vertices into buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBufferBytes, spvertices.data());

    // bind indices to buffer
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferBytes,
                 modelData.indices.data(), GL_STATIC_DRAW);

    try {
        pCubemapTex = std::make_unique<CubemapTexture>(
            Directory, PosXFilename, NegXFilename, PosYFilename, NegYFilename,
            PosZFilename, NegZFilename);
    } catch (const std::bad_alloc&) {
        spdlog::error("Error creating new Cubemap");
        return false;
    }
    pCubemapTex->Load();

    shaderProgram->Use();
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
