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
    shader->AddShader(ReadFile("shaders/billboard.vsh"), VertexShader);
    shader->AddShader(ReadFile("shaders/billboard.fsh"), FragmnetShader);
    shader->AddShader(ReadFile("shaders/billboard.gsh"), GeometryShader);
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
    colorMap.Bind(GL_TEXTURE0);
    colSamplerID = shaderProgram->GetUniformLocation("colTexSampler");
    glUniform1i(colSamplerID, 0);

    glGenBuffers(1, &VBO);
    if (VBO == 0) {
        spdlog::error("Unable to create billboard vertex buffer");
        throw std::runtime_error("Unable to create billboard vertex buffer");
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // creating a buffer
    const std::array<float, 3> coords = {Pos.x, Pos.y, Pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords.data(),
                 GL_DYNAMIC_DRAW);
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    camViewID = shaderProgram->GetUniformLocation("gVP");
    camPosID = shaderProgram->GetUniformLocation("gCameraPos");
}

Billboard::~Billboard() = default;
void Billboard::SetPos(glm::vec3 pos) {
    this->Pos = pos;
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // creating a buffer
    const std::array<float, 3> coords = {pos.x, pos.y, pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords.data(),
                 GL_DYNAMIC_DRAW);
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
}

void Billboard::Render(const Camera& cam) { Draw(cam, Pos); }

void Billboard::Draw(const Camera& cam, const glm::vec3& position) const {
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    const glm::mat4 viewProjection = cam.GetViewProjectionMatrix();

    // camera projection matrix
    glUniformMatrix4fv(camViewID, 1, GL_FALSE, glm::value_ptr(viewProjection));
    // camera position
    glUniform3f(camPosID, cam.GetPos().x, cam.GetPos().y, cam.GetPos().z);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    const std::array<float, 3> coords = {position.x, position.y, position.z};
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(coords), coords.data());
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    colorMap.Bind(GL_TEXTURE0);
    glUniform1i(colSamplerID,
                0);  // tell shader to use as texture 0

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionID);
    glDrawArrays(GL_POINTS, 0, 1);
    glDisableVertexAttribArray(positionID);
}
