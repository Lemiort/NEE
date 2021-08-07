#include "Light.h"

#include <spdlog/spdlog.h>

#include <fstream>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

void DirectionalLight::SetCol(glm::vec3 col) {
    color.x = col.x;
    color.y = col.y;
    color.z = col.z;
}

void DirectionalLight::SetDir(glm::vec3 dir) {
    direction[0] = dir.x;
    direction[1] = dir.y;
    direction[2] = dir.z;
}

DirectionalLight::DirectionalLight(GLfloat d1, GLfloat d2, GLfloat d3,
                                   GLfloat r, GLfloat g, GLfloat b,
                                   std::shared_ptr<Material> _mat) {
    direction[0] = d1;
    direction[1] = d2;
    direction[2] = d3;
    direction[3] = 1;
    color.x = r;
    color.y = g;
    color.z = b;

    mesh = std::make_shared<Mesh>();
    // mesh->Init(_mat,"models/quad2x2front.ho3d");
    mesh->Init(_mat, "models/cube2x2x2.ho3d");
}

void DirectionalLight::Render(const Camera& cam) {
    // mesh->SetScale(1.0,1.0,1.0);
    mesh->SetPosition({0, 0, 0});
    mesh->Render(cam);
}

glm::vec3 DirectionalLight::GetDir() {
    return glm::vec3(direction[0], direction[1], direction[2]);
}
glm::vec3 DirectionalLight::GetCol() { return color; }

glm::vec3 SpotLight::GetPos() {
    return glm::vec3(position[0], position[1], position[2]);
}

SpotLight::SpotLight(GLfloat t1, GLfloat t2, GLfloat t3, GLfloat r, GLfloat g,
                     GLfloat b, float p1, float p2, float p3, float cut,
                     std::shared_ptr<Material> _mat) {
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
    cutoff_angle = cut;

    mesh = std::make_shared<Mesh>();
    mesh->Init(_mat, "models/cone2.ho3d");
    mesh->SetRotation(90, 0, 0);
}

void SpotLight::Render(const Camera& cam) {
    direction[0] = target[0] - position[0];
    direction[1] = target[1] - position[1];
    direction[2] = target[2] - position[2];
    direction[3] = 1;

    glm::vec3 dir(direction[0], direction[1], direction[2]);
    glm::vec3 proj(0, 1, 0);
    glm::vec3 directionXY(direction[0], direction[1], 0);
    glm::vec3 directionYZ(0, direction[1], direction[2]);
    glm::vec3 directionXZ(direction[0], 0, direction[2]);

    float x_rotation = 0, y_rotation = 0, z_rotation = 0;

    // X axis
    {
        // get the projection
        glm::vec3 projYZ(0, proj.y, proj.z);

        // calc model_rotation
        if (glm::cross(projYZ, directionYZ).x > 0)
            x_rotation = -glm::degrees(glm::angle(directionYZ, projYZ));
        else
            x_rotation = glm::degrees(glm::angle(directionYZ, projYZ));

        // update up vector
        proj = glm::rotate(proj, -x_rotation, glm::vec3(1, 0, 0));
        spdlog::debug("proj x: ({}, {}, {})", proj.x, proj.y, proj.z);
    }

    // Y axis
    if (proj != dir) {
        // get the projection
        glm::vec3 projXZ(proj.x, 0, proj.z);

        // calc the model_rotation
        if (glm::cross(projXZ, directionXZ).y > 0)
            y_rotation = -glm::degrees(glm::angle(directionXZ, projXZ));
        else
            y_rotation = glm::degrees(glm::angle(directionXZ, projXZ));

        // update the up vector
        proj = glm::rotate(proj, -y_rotation, glm::vec3(0, 1, 0));
        spdlog::debug("proj y: ({}, {}, {})", proj.x, proj.y, proj.z);

        // Z axis
        if (proj != dir) {
            // get the projection
            glm::vec3 projXY(proj.x, proj.y, 0);

            // calc the model_rotation
            if (glm::cross(projXY, directionXY).z > 0)
                z_rotation = glm::degrees(glm::angle(directionXY, projXY));
            else
                z_rotation = -glm::degrees(glm::angle(directionXY, projXY));
            proj = glm::rotate(proj, -z_rotation, glm::vec3(0, 0, 1));
            spdlog::debug("proj z: ({}, {}, {})", proj.x, proj.y, proj.z);
        }
    }

    mesh->SetRotation(x_rotation, y_rotation, z_rotation);

    /*//==========spherical coordinates solution=======
    glm::vec3 sphericalDirection = ToSphericalCoordinates( dir );
    glm::vec3 sphericalUp = ToSphericalCoordinates( glm::vec3(0.0,
                                                            1.0,
                                                            0.0) );
    mesh->SetRotate( sphericalDirection.y-sphericalUp.y,
                    0.0f,
                    sphericalDirection.z - sphericalUp.z);
    //=========================================================*/

    // =====vector rotattion solution============
    // glm::vec3 rotateVector =  dir.Cross(proj);
    /*mesh->SetVectorRotate(rotateVector,
                          glm::angle(dir,proj));*/
    // mesh->SetVectorRotate(proj,
    //                       -90.0f);
    //==========================================================

    mesh->SetScale(glm::length(dir) * cos(glm::radians(cutoff_angle)),
                   glm::length(dir),
                   glm::length(dir) * cos(glm::radians(cutoff_angle)));

    mesh->SetPosition(position);
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
    cutoff_angle = cut;
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
                       float p, std::shared_ptr<Material> _mat) {
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

    sphere = std::make_shared<Mesh>();
    sphere->Init(_mat, "models/normal_geosphere.ho3d");
    radius = CalcSphereSize() / 2;
    spdlog::debug("Light radius {}", radius);
}

void PointLight::Render(const Camera& cam) {
    sphere->SetScale(radius, radius, radius);
    sphere->SetPosition(position);
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
    return 8.0f * sqrtf(glm::length(color) * power) + 1.0f;
}

Line::Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color) {
    std::string vertex_shader_text{ReadFromFile("shaders/lightVS.vs")};
    std::string fragment_shader_text{ReadFromFile("shaders/lightFS.fs")};

    shaderProgram = std::make_shared<Shader>();
    shaderProgram->AddShader(vertex_shader_text, kVertexShader);
    shaderProgram->AddShader(fragment_shader_text, kFragmentShader);
    shaderProgram->Init();

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
    position_id = shaderProgram->GetAttribLocation("position");
    model_id = shaderProgram->GetUniformLocation("model");
    PixelColorID = shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID = shaderProgram->GetUniformLocation("size");
}
Line::Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color,
           std::shared_ptr<Shader> shader) {
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
    position_id = shaderProgram->GetAttribLocation("position");
    model_id = shaderProgram->GetUniformLocation("model");
    PixelColorID = shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID = shaderProgram->GetUniformLocation("size");
}

// void Line::Render(Camera* pGameCamera, int width, int height)
void Line::Render(const Camera& cam) {
    glm::mat4 model =
        glm::mat4() * glm::diagonal4x4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    model = glm::transpose(model);

    shaderProgram->Use();
    // glUseProgram(shaderProgramID);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glUniformMatrix4fv(model_id, 1, GL_TRUE, glm::value_ptr(model));
    glUniform1f(PointSizeID, 1.5);
    glUniform4f(PixelColorID, col[0], col[1], col[2], 1.0);
    glEnableVertexAttribArray(position_id);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableVertexAttribArray(position_id);
}
std::shared_ptr<Shader> Line::GetShader() { return shaderProgram; }
