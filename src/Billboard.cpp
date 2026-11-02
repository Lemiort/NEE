#include "Billboard.hpp"

#include <glad/gl.h>

#include <array>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Camera.hpp"
#include "Shader.hpp"
#include "spdlog/spdlog.h"

namespace {
std::shared_ptr<Shader> CreateBillboardShader() {
    auto shader = std::make_shared<Shader>();
    shader->AddShader(ReadFile("shaders/billboard.vsh"), ShaderType::Vertex);
    shader->AddShader(ReadFile("shaders/billboard.fsh"), ShaderType::Fragment);
    shader->AddShader(ReadFile("shaders/billboard.gsh"), ShaderType::Geometry);
    shader->Link();
    return shader;
}
}  // namespace

Billboard::Billboard(const std::string& textureFilename)
    : Billboard(gsl_lite::not_null{CreateBillboardShader()}, textureFilename) {}

Billboard::Billboard(gsl_lite::not_null<std::shared_ptr<Shader>> shader,
                     const std::string& textureFilename)
    : Pos(0.0F) {
    shaderProgram = std::move(shader);
    if (!colorMap.Load(textureFilename)) {
        spdlog::error("Unable to load billboard texture '{}'", textureFilename);
        throw std::runtime_error("Unable to load billboard texture: " +
                                 textureFilename);
    }
    colSamplerID = shaderProgram->GetUniformLocation("colTexSampler");
    glProgramUniform1i(shaderProgram->GetShaderProgramId(), colSamplerID, 0);

    glCreateBuffers(1, &VBO);
    if (VBO == 0) {
        spdlog::error("Unable to create billboard vertex buffer");
        throw std::runtime_error("Unable to create billboard vertex buffer");
    }
    const std::array<float, 3> coords = {Pos.x, Pos.y, Pos.z};
    glNamedBufferStorage(VBO, sizeof(coords), coords.data(),
                         GL_DYNAMIC_STORAGE_BIT);
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    vertexArray_.SetBuffer(0, VBO, 3 * sizeof(float));
    vertexArray_.SetAttribute(positionID, 0, 3);
    camViewID = shaderProgram->GetUniformLocation("gVP");
    camPosID = shaderProgram->GetUniformLocation("gCameraPos");
}

Billboard::~Billboard() = default;
void Billboard::SetPos(glm::vec3 pos) {
    this->Pos = pos;
    const std::array<float, 3> coords = {pos.x, pos.y, pos.z};
    glNamedBufferSubData(VBO, 0, sizeof(coords), coords.data());
}

void Billboard::Render(const Camera& cam) { Draw(cam, Pos); }

void Billboard::Draw(const Camera& cam, const glm::vec3& position) const {
    shaderProgram->Use();
    const glm::mat4 viewProjection = cam.GetViewProjectionMatrix();

    glUniformMatrix4fv(camViewID, 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform3f(camPosID, cam.GetPos().x, cam.GetPos().y, cam.GetPos().z);

    const std::array<float, 3> coords = {position.x, position.y, position.z};
    glNamedBufferSubData(VBO, 0, sizeof(coords), coords.data());

    colorMap.Bind(GL_TEXTURE0);
    glUniform1i(colSamplerID,
                0);  // tell shader to use as texture 0

    vertexArray_.Bind();
    glDrawArrays(GL_POINTS, 0, 1);
}
