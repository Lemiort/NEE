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
#include <stdexcept>
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

SkyBox::SkyBox(gsl_lite::not_null<std::shared_ptr<Shader>> shader,
               const std::string& directory, const std::string& positiveX,
               const std::string& negativeX, const std::string& positiveY,
               const std::string& negativeY, const std::string& positiveZ,
               const std::string& negativeZ)
    : shaderProgramID(shader->GetShaderProgramId()) {
    shaderProgram = shader;
    Scale = 0;
    ModelData modelData;
    std::vector<float> spvertices;
    spfaces = 0;
    spverts = 0;
    try {
        if (!LoadModel("models/sphere.obj", modelData)) {
            throw std::runtime_error("Unable to load skybox model");
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
            throw std::length_error("Skybox model exceeds supported limits");
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
        throw;
    }
    pCubemapTex = std::make_unique<CubemapTexture>(
        directory, positiveX, negativeX, positiveY, negativeY, positiveZ,
        negativeZ);
    if (!pCubemapTex->Load()) {
        spdlog::error("Unable to load required skybox cubemap");
        throw std::runtime_error("Unable to load skybox cubemap");
    }
    // create buffer to store everything
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    const auto vertexBufferBytes =
        static_cast<GLsizeiptr>(sizeof(float) * kPositionComponentCount *
                                static_cast<std::size_t>(spverts));
    const auto indexBufferBytes =
        static_cast<GLsizeiptr>(sizeof(std::uint32_t) * kTriangleIndexCount *
                                static_cast<std::size_t>(spfaces));
    glBufferData(GL_ARRAY_BUFFER, vertexBufferBytes, nullptr, GL_STATIC_DRAW);
    // load vertices into buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBufferBytes, spvertices.data());

    // bind indices to buffer
    glGenBuffers(1, &IBO);
    if (VBO == 0 || IBO == 0) {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &IBO);
        VBO = 0;
        IBO = 0;
        spdlog::error("Unable to create skybox GPU buffers");
        throw std::runtime_error("Unable to create skybox GPU buffers");
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferBytes,
                 modelData.indices.data(), GL_STATIC_DRAW);

    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    // find uniform variable addresses
    gWorldID = shaderProgram->GetUniformLocation("gWVP");

    // do the same thing
    pCubemapTex->Bind(GL_TEXTURE2);
    textureID = shaderProgram->GetUniformLocation("gCubemapTexture");
    glActiveTexture(GL_TEXTURE2);
    glUniform1i(textureID, 2);
    transform_ = Transform{};
}

SkyBox::~SkyBox() = default;
void SkyBox::Render(const Camera& cam) { Draw(cam); }

void SkyBox::Draw(const Camera& cam) const {
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
                   static_cast<GLsizei>(static_cast<std::size_t>(spfaces) *
                                        kTriangleIndexCount),
                   GL_UNSIGNED_INT, nullptr);
    glDisableVertexAttribArray(positionID);

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
}
