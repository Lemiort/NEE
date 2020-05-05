#include "Billboard.h"

#include <glm/gtc/type_ptr.hpp>

Billboard::Billboard() {
    Pos = glm::vec3(1, 1, 1);
    shader = false;
}

/*Billboard::Billboard(GLuint _shader)
{
    Pos=glm::vec3(0,0,0);
    shaderProgramID=_shader;
    shader=true;
}*/

Billboard::Billboard(std::shared_ptr<Shader> _shader) {
    Pos = glm::vec3(0, 0, 0);
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
    float coords[3] = {Pos.x, Pos.y, Pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (3), coords, GL_DYNAMIC_DRAW);
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    camViewID = shaderProgram->GetUniformLocation("gVP");
    camPosID = shaderProgram->GetUniformLocation("gCameraPos");
}
void Billboard::SetPos(glm::vec3 _Pos) {
    Pos = _Pos;
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // создаём буффер
    float coords[3] = {Pos.x, Pos.y, Pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (3), coords, GL_DYNAMIC_DRAW);
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
}

void Billboard::Render(Camera* cam) {
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    Assistant TM;  // TM - Для объекта, 2- для нормали объекта, 3 - для позиции
                   // камера для спекуляра
    TM.SetCamera(cam->GetPos(), cam->GetTarget(), cam->GetUp());
    TM.SetPerspectiveProj(cam->GetFov(), cam->GetWidth(), cam->GetHeight(),
                          cam->GetZNear(), cam->GetZFar());

    // матрица проекции камеры
    glUniformMatrix4fv(camViewID, 1, GL_TRUE, glm::value_ptr(TM.GetVC()));
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
