#include "Billboard.hpp"

#include <glad/gl.h>

#include <memory>
#include <utility>

#include "Assistant.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "ShaderFunctions.hpp"

Billboard::Billboard() : shader(false) { Pos = Vector3f(1, 1, 1); }

/*Billboard::Billboard(GLuint _shader)
{
    Pos=Vector3f(0,0,0);
    shaderProgramID=_shader;
    shader=true;
}*/

Billboard::Billboard(shared_ptr<Shader> _shader) : shader(true) {
    Pos = Vector3f(0, 0, 0);
    shaderProgram = std::move(_shader);
    // shaderProgramID=_shader->shaderProgramID;
}

Billboard::~Billboard() {
    // dtor
}

void Billboard::Init(const char* TexFilename) {
    if (!shader) {
        char const* vertexShaderSorceCode = ReadFile("shaders/billboard.vsh");
        char const* fragmentShaderSourceCode =
            ReadFile("shaders/billboard.fsh");
        char const* geometryShaderSourceCode =
            ReadFile("shaders/billboard.gsh");
        shaderProgram = make_shared<Shader>();
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
    // creating a buffer
    float coords[3] = {Pos.x, Pos.y, Pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, coords, GL_DYNAMIC_DRAW);
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
    camViewID = shaderProgram->GetUniformLocation("gVP");
    camPosID = shaderProgram->GetUniformLocation("gCameraPos");
}
void Billboard::SetPos(Vector3f Pos) {
    Pos = Pos;
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // creating a buffer
    float coords[3] = {Pos.x, Pos.y, Pos.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, coords, GL_DYNAMIC_DRAW);
    positionID = shaderProgram->GetAttribLocation("s_vPosition");
}

void Billboard::Render(const Camera& cam) {
    // glUseProgram(shaderProgramID);
    shaderProgram->Use();
    Assistant TM;  // TM - For object, 2 - for object normal, 3 - for position
                   // camera for specular
    TM.SetCamera(cam.GetPos(), cam.GetTarget(), cam.GetUp());
    TM.SetPerspectiveProj(cam.GetFov(), cam.GetWidth(), cam.GetHeight(),
                          cam.GetZNear(), cam.GetZFar());

    // camera projection matrix
    glUniformMatrix4fv(camViewID, 1, GL_TRUE,
                       reinterpret_cast<const GLfloat*>(TM.GetVC()));
    // camera position
    glUniform3f(camPosID, cam.GetPos().x, cam.GetPos().y, cam.GetPos().z);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
