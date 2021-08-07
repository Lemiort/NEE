#include "Billboard.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/transform.hpp>

#include "ShaderFunctions.h"

Billboard::Billboard() {
    pos = glm::vec3(1, 1, 1);
    shader = false;
}

/*Billboard::Billboard(GLuint _shader)
{
    Pos=glm::vec3(0,0,0);
    shaderProgramID=_shader;
    shader=true;
}*/

Billboard::Billboard(std::shared_ptr<Shader> _shader) {
    pos = glm::vec3(0, 0, 0);
    shaderProgram = _shader;
    // shaderProgramID=_shader->shaderProgramID;
    shader = true;
}

Billboard::~Billboard() {
    // dtor
}

void Billboard::Init(const char* TexFilename) {
    if (shader == false) {
        std::string vertex_shader_text{ReadFromFile("shaders/billboard.vs")};
        std::string fragment_shader_text{ReadFromFile("shaders/billboard.fs")};
        std::string geometry_shader_text{ReadFromFile("shaders/billboard.gs")};

        shaderProgram = std::make_shared<Shader>();
        shaderProgram->AddShader(vertex_shader_text, kVertexShader);
        shaderProgram->AddShader(fragment_shader_text, kFragmentShader);
        shaderProgram->AddShader(geometry_shader_text, kGeometryShader);
        shaderProgram->Init();
        shader = true;
    }
    colorMap.Load(TexFilename);
    colorMap.Bind(GL_TEXTURE0);
    colSamplerID = shaderProgram->GetUniformLocation("colTexSampler");
    glUniform1i(colSamplerID, 0);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // create buffer
    float coords[3] = {pos.x, pos.y, pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (3), coords, GL_DYNAMIC_DRAW);
    position_id = shaderProgram->GetAttribLocation("vertex_position");
    mvp_id = shaderProgram->GetUniformLocation("gVP");
    camPosID = shaderProgram->GetUniformLocation("gCameraPos");
}
void Billboard::SetPos(glm::vec3 _Pos) {
    pos = _Pos;
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // create buffer
    float coords[3] = {pos.x, pos.y, pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (3), coords, GL_DYNAMIC_DRAW);
    position_id = shaderProgram->GetAttribLocation("vertex_position");
}

void Billboard::Render(const Camera& cam) {
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();

    glm::mat4 model = glm::translate(pos) *
                      glm::diagonal4x4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    model = glm::transpose(model);

    glm::mat4 projection = glm::perspectiveFov(
        cam.GetFov(), static_cast<float>(cam.GetWidth()),
        static_cast<float>(cam.GetHeight()), cam.GetZNear(), cam.GetZFar());
    glm::mat4 view = glm::lookAt(cam.GetPos(), cam.GetTarget(), cam.GetUp());
    glm::mat4 vp_matrix = glm::transpose(projection * view);
    glm::mat4 mvp_matrix = vp_matrix * model;

    glUniformMatrix4fv(mvp_id, 1, GL_TRUE, glm::value_ptr(mvp_matrix));
    glUniform3f(camPosID, cam.GetPos().x, cam.GetPos().y, cam.GetPos().z);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    colorMap.Bind(GL_TEXTURE0);
    glUniform1i(colSamplerID, 0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(position_id);
    glDrawArrays(GL_POINTS, 0, 1);
    glDisableVertexAttribArray(position_id);
}
