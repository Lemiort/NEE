#include "Billboard.h"

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
        char* vertexShaderSorceCode = ReadFile("shaders/billboard.vs");
        char* fragmentShaderSourceCode = ReadFile("shaders/billboard.fs");
        char* geometryShaderSourceCode = ReadFile("shaders/billboard.gs");
        shaderProgram = std::make_shared<Shader>();
        shaderProgram->AddShader(vertexShaderSorceCode, VertexShader);
        shaderProgram->AddShader(fragmentShaderSourceCode, FragmnetShader);
        shaderProgram->AddShader(geometryShaderSourceCode, GeometryShader);
        shaderProgram->Init();
        /*GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
        GLuint fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode);
        GLuint geometryShaderID=MakeGeometryShader(geometryShaderSourceCode);
        shaderProgramID=MakeShaderProgram(vertexShaderID,geometryShaderID,
        fragmentShaderID);*/
        // shaderProgramID=MakeShaderProgram(vertexShaderID, fragmentShaderID);
        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
        delete[] geometryShaderSourceCode;
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
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
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
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
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
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    colorMap.Bind(GL_TEXTURE0);
    glUniform1i(colSamplerID,
                0);  // говорим шейдеру, чтобы использовал в качестве текстуры 0

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionID);
    glDrawArrays(GL_POINTS, 0, 1);
    glDisableVertexAttribArray(positionID);
}
