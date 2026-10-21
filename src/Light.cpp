#include "Light.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

#include "spdlog/spdlog.h"

void DirectionalLight::SetCol(glm::vec3 col) {
    color[0] = col.x;
    color[1] = col.y;
    color[2] = col.z;
}

void DirectionalLight::SetDir(glm::vec3 dir) {
    direction[0] = dir.x;
    direction[1] = dir.y;
    direction[2] = dir.z;
}

DirectionalLight::DirectionalLight(GLfloat d1, GLfloat d2, GLfloat d3,
                                   GLfloat r, GLfloat g, GLfloat b,
                                   shared_ptr<Material> _mat) {
    direction[0] = d1;
    direction[1] = d2;
    direction[2] = d3;
    direction[3] = 1;
    color[0] = r;
    color[1] = g;
    color[2] = b;

    mesh = make_shared<Mesh>();
    // mesh->Init(_mat,"models/quad2x2front.ho3d");
    mesh->Init(_mat, "models/cube2x2x2.ho3d");
}

void DirectionalLight::Render(const Camera& cam) {
    // mesh->SetScale(1.0,1.0,1.0);
    mesh->SetPosition(0, 0, 0);
    mesh->Render(cam);
}

glm::vec3 DirectionalLight::GetDir() {
    return glm::vec3(direction[0], direction[1], direction[2]);
}
glm::vec3 DirectionalLight::GetCol() {
    return glm::vec3(color[0], color[1], color[2]);
}

DirectionalLight::DirectionalLight() {}
DirectionalLight::~DirectionalLight() {}

glm::vec3 SpotLight::GetPos() {
    return glm::vec3(position[0], position[1], position[2]);
}

SpotLight::SpotLight() {}

SpotLight::SpotLight(GLfloat t1, GLfloat t2, GLfloat t3, GLfloat r, GLfloat g,
                     GLfloat b, float p1, float p2, float p3, float cut,
                     shared_ptr<Material> _mat) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
    position[0] = p1;
    position[1] = p2;
    position[2] = p3;
    target[0] = t1;
    target[1] = t2;
    target[2] = t3;
    direction[0] = t1 - p1;
    direction[1] = t2 - p2;
    direction[2] = t3 - p3;
    direction[3] = 1;
    Cutoff = cut;

    mesh = make_shared<Mesh>();
    mesh->Init(_mat, "models/cone2.ho3d");
}

void SpotLight::Render(const Camera& cam) {
    const glm::vec3 dir(target[0] - position[0], target[1] - position[1],
                       target[2] - position[2]);
    const float distance = glm::length(dir);
    if (distance <= 1e-6f) return;

    direction[0] = dir.x;
    direction[1] = dir.y;
    direction[2] = dir.z;
    direction[3] = 1.0f;

    // The cone mesh points along +Y. Align its axis with the target direction.
    const glm::vec3 unit = dir / distance;
    const float x =
        glm::degrees(glm::asin(glm::clamp(unit.z, -1.0f, 1.0f)));
    const float z = glm::abs(unit.z) >= 1.0f - 1e-6f
                        ? 0.0f
                        : glm::degrees(glm::atan(-unit.x, unit.y));
    mesh->SetRotation(x, 0.0f, z);

    // Cutoff is the cone's half angle; its radius is distance * tan(cutoff).
    const float radius = distance * glm::tan(glm::radians(Cutoff));
    mesh->SetScale(radius, distance, radius);
    mesh->SetPosition(position[0], position[1], position[2]);
    mesh->Render(cam);
}

void SpotLight::SetTarget(glm::vec3 _target) {
    target[0] = _target.x;
    target[1] = _target.y;
    target[2] = _target.z;
    direction[0] = target[0] - position[0];
    direction[1] = target[1] - position[1];
    direction[2] = target[2] - position[2];
    direction[3] = 1;
}

SpotLight::~SpotLight() {}
void SpotLight::Init(GLfloat d1, GLfloat d2, GLfloat d3, GLfloat r, GLfloat g,
                     GLfloat b, float p1, float p2, float p3, float cut) {
    direction[0] = d1;
    direction[1] = d2;
    direction[2] = d3;
    direction[3] = 1;
    color[0] = r;
    color[1] = g;
    color[2] = b;
    position[0] = p1;
    position[1] = p2;
    position[2] = p3;
    Cutoff = cut;
}

void SpotLight::SetPos(glm::vec3 pos) {
    position[0] = pos.x;
    position[1] = pos.y;
    position[2] = pos.z;

    direction[0] = target[0] - position[0];
    direction[1] = target[1] - position[1];
    direction[2] = target[2] - position[2];
    direction[3] = 1;
}

PointLight::PointLight(float d1, float d2, float d3, float r, float g, float b,
                       float p, shared_ptr<Material> _mat) {
    position[0] = d1;
    position[1] = d2;
    position[2] = d3;
    color[0] = r;
    color[1] = g;
    color[2] = b;
    power = p;
    temp[0] = 0;
    temp[1] = 0;
    temp[2] = 0;
    /*char* vertexShaderSorceCode=ReadFile("shaders/lightVS.vsh");
    char* fragmentShaderSourceCode=ReadFile("shaders/lightFS.fsh");
    shaderProgram = std::make_unique<Shader>();
    shaderProgram->AddShader(vertexShaderSorceCode,VertexShader);
    shaderProgram->AddShader(fragmentShaderSourceCode,FragmnetShader);
    shaderProgram->Init();

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(temp),NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(temp),temp);
    positionID=	    shaderProgram->GetAttribLocation("position");
    gWorldID=	    shaderProgram->GetUniformLocation("gWVCP");
    PixelColorID=   shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID=    shaderProgram->GetUniformLocation("size");*/

    sphere = make_shared<Mesh>();
    sphere->Init(_mat, "models/normal_geosphere.ho3d");
    radius = CalcSphereSize() / 2;
    spdlog::info("Light radius is {}", radius);
}
PointLight::~PointLight() {}
void PointLight::Render(const Camera& cam) {
    /*Assistant TM;
    TM.WorldPos(position[0],position[1],position[2]);
    TM.SetCamera(cam.GetPos(), cam.GetTarget(), cam.GetUp());
    TM.SetPerspectiveProj(cam, Width, Height, zNear, zFar);*/

    /*shaderProgram->Use();
                    //glUseProgram(shaderProgramID);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    glVertexAttribPointer(positionID,3,GL_FLOAT,GL_FALSE,0,0);


                    glUniformMatrix4fv(gWorldID, 1, GL_FALSE, (const
       GLfloat*)TM.GetTSRVC());
                    glUniform4f(PixelColorID,color[0],color[1],color[2],1);
                    glUniform1f(PointSizeID, 5.0);
                    glEnableVertexAttribArray(positionID);
                    glDrawArrays(GL_POINTS,0,1);
                    glDisableVertexAttribArray(positionID);*/

    sphere->SetScale(radius, radius, radius);
    sphere->SetPosition(position[0], position[1], position[2]);
    sphere->Render(cam);
}

void PointLight::SetPos(glm::vec3 pos) {
    position[0] = pos.x;
    position[1] = pos.y;
    position[2] = pos.z;
}

void PointLight::SetCol(glm::vec3 col) {
    color[0] = col.x;
    color[1] = col.y;
    color[2] = col.z;

    radius = CalcSphereSize();
}
float PointLight::CalcSphereSize() {
    return 8.0f *
               sqrtf(glm::length(glm::vec3(color[0], color[1], color[2])) * power) +
           1.0f;
}

Line::Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color) {
    shaderProgram = make_shared<Shader>();
    shaderProgram->AddShader(ReadFile("shaders/lightVS.vsh"), VertexShader);
    shaderProgram->AddShader(ReadFile("shaders/lightFS.fsh"), FragmnetShader);
    shaderProgram->Init();
    /*GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
    GLuint fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode);
    shaderProgramID=MakeShaderProgram(vertexShaderID, fragmentShaderID);*/

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_STATIC_DRAW);

    pos[0] = pos1.x;
    pos[1] = pos1.y;
    pos[2] = pos1.z;
    pos[3] = pos2.x;
    pos[4] = pos2.y;
    pos[5] = pos2.z;

    col[0] = color.x;
    col[1] = color.y;
    col[2] = color.z;
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
    positionID = shaderProgram->GetAttribLocation("position");
    gWorldID = shaderProgram->GetUniformLocation("gWVCP");
    PixelColorID = shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID = shaderProgram->GetUniformLocation("size");
}
Line::Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color,
           shared_ptr<Shader> shader) {
    // shaderProgramID=shader;
    shaderProgram = shader;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_STATIC_DRAW);

    pos[0] = pos1.x;
    pos[1] = pos1.y;
    pos[2] = pos1.z;
    pos[3] = pos2.x;
    pos[4] = pos2.y;
    pos[5] = pos2.z;

    col[0] = color.x;
    col[1] = color.y;
    col[2] = color.z;
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
    positionID = shaderProgram->GetAttribLocation("position");
    gWorldID = shaderProgram->GetUniformLocation("gWVCP");
    PixelColorID = shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID = shaderProgram->GetUniformLocation("size");
}
Line::~Line() {}
// void Line::Render(const Camera& pGameCamera, int width, int height)
void Line::Render(const Camera& cam) {
    Assistant TM;
    TM.WorldPos(0, 0, 0);
    TM.SetCamera(cam.GetPos(), cam.GetTarget(), cam.GetUp());
    TM.SetPerspectiveProj(cam.GetFov(), cam.GetWidth(), cam.GetHeight(),
                          cam.GetZNear(), cam.GetZFar());

    shaderProgram->Use();
    // glUseProgram(shaderProgramID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glUniformMatrix4fv(gWorldID, 1, GL_FALSE, glm::value_ptr(*TM.GetTSRVC()));
    glUniform1f(PointSizeID, 1.5);
    glUniform4f(PixelColorID, col[0], col[1], col[2], 1.0);
    glEnableVertexAttribArray(positionID);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableVertexAttribArray(positionID);
}
shared_ptr<Shader> Line::GetShader() { return shaderProgram; }
