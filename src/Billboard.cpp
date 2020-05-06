#include "Billboard.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/transform.hpp>

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
        std::ifstream vertex_shader_file("shaders/billboard.vs");
        std::string vertex_shader_text(
            (std::istreambuf_iterator<char>(vertex_shader_file)),
            (std::istreambuf_iterator<char>()));

        std::ifstream fragment_shader_file("shaders/billboard.fs");
        std::string fragment_shader_text(
            (std::istreambuf_iterator<char>(fragment_shader_file)),
            (std::istreambuf_iterator<char>()));

        std::ifstream geomentry_shader_file("shaders/billboard.gs");
        std::string geometry_shader_text(
            (std::istreambuf_iterator<char>(geomentry_shader_file)),
            (std::istreambuf_iterator<char>()));

        shaderProgram = std::make_shared<Shader>();
        shaderProgram->AddShader(vertex_shader_text, VertexShader);
        shaderProgram->AddShader(fragment_shader_text, FragmnetShader);
        shaderProgram->AddShader(geometry_shader_text, GeometryShader);
        shaderProgram->Init();
        shader = true;
    }
    colorMap.Load(TexFilename);
    colorMap.Bind(GL_TEXTURE0);
    colSamplerID = shaderProgram->GetUniformLocation("colTexSampler");
    glUniform1i(colSamplerID, 0);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // создаём буффер
    float coords[3] = {pos.x, pos.y, pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (3), coords, GL_DYNAMIC_DRAW);
    position_id = shaderProgram->GetAttribLocation("vertex_position");
    camViewID = shaderProgram->GetUniformLocation("gVP");
    camPosID = shaderProgram->GetUniformLocation("gCameraPos");
}
void Billboard::SetPos(glm::vec3 _Pos) {
    pos = _Pos;
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // создаём буффер
    float coords[3] = {pos.x, pos.y, pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (3), coords, GL_DYNAMIC_DRAW);
    position_id = shaderProgram->GetAttribLocation("vertex_position");
}

void Billboard::Render(Camera* cam) {
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();

    glm::mat4 model = glm::translate(pos) *
                      glm::diagonal4x4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    glm::mat4 projection = glm::perspectiveFov(
        cam->GetFov(), static_cast<float>(cam->GetWidth()),
        static_cast<float>(cam->GetHeight()), cam->GetZNear(), cam->GetZFar());
    glm::mat4 view = glm::lookAt(cam->GetPos(), cam->GetTarget(), cam->GetUp());
    glm::mat4 mvp_matrix = projection * view * model;

    // матрица проекции камеры
    glUniformMatrix4fv(camViewID, 1, GL_TRUE, glm::value_ptr(mvp_matrix));
    // позиция камеры
    glUniform3f(camPosID, cam->GetPos().x, cam->GetPos().y, cam->GetPos().z);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    colorMap.Bind(GL_TEXTURE0);
    glUniform1i(colSamplerID,
                0);  // говорим шейдеру, чтобы использовал в качестве текстуры 0

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(position_id);
    glDrawArrays(GL_POINTS, 0, 1);
    glDisableVertexAttribArray(position_id);
}
