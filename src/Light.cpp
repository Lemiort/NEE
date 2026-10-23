#include "Light.hpp"

#include <glad/gl.h>

#include <cmath>
#include <glm/common.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <utility>

#include "Camera.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "spdlog/spdlog.h"

namespace {
float CalculateSphereSize(float red, float green, float blue, float power) {
    return (8.0F * sqrtf(glm::length(glm::vec3(red, green, blue)) * power)) +
           1.0F;
}
}  // namespace

void DirectionalLight::SetCol(glm::vec3 col) {
    color.at(0) = col.x;
    color.at(1) = col.y;
    color.at(2) = col.z;
}

void DirectionalLight::SetDir(glm::vec3 dir) {
    direction.at(0) = dir.x;
    direction.at(1) = dir.y;
    direction.at(2) = dir.z;
}

DirectionalLight::DirectionalLight(GLfloat d1, GLfloat d2, GLfloat d3,
                                   GLfloat r, GLfloat g, GLfloat b,
                                   shared_ptr<Material> _mat) {
    direction.at(0) = d1;
    direction.at(1) = d2;
    direction.at(2) = d3;
    direction.at(3) = 1;
    color.at(0) = r;
    color.at(1) = g;
    color.at(2) = b;

    mesh = std::make_shared<Mesh>(gsl_lite::not_null{std::move(_mat)},
                                  "models/cube2x2x2.obj");
}

void DirectionalLight::Render(const Camera& cam) {
    // mesh->SetScale(1.0,1.0,1.0);
    mesh->SetPosition(0, 0, 0);
    mesh->Render(cam);
}

glm::vec3 DirectionalLight::GetDir() {
    return {direction.at(0), direction.at(1), direction.at(2)};
}
glm::vec3 DirectionalLight::GetCol() {
    return {color.at(0), color.at(1), color.at(2)};
}

DirectionalLight::DirectionalLight() = default;
DirectionalLight::~DirectionalLight() = default;

glm::vec3 SpotLight::GetPos() { return GetPosition(); }

glm::vec3 SpotLight::GetDirection() const {
    return glm::vec3(target.at(0), target.at(1), target.at(2)) - GetPosition();
}

SpotLight::SpotLight() = default;

SpotLight::SpotLight(GLfloat t1, GLfloat t2, GLfloat t3, GLfloat r, GLfloat g,
                     GLfloat b, float p1, float p2, float p3, float cut,
                     shared_ptr<Material> _mat)
    : Cutoff(cut) {
    color.at(0) = r;
    color.at(1) = g;
    color.at(2) = b;
    SetPosition(p1, p2, p3);
    target.at(0) = t1;
    target.at(1) = t2;
    target.at(2) = t3;
    direction.at(0) = t1 - p1;
    direction.at(1) = t2 - p2;
    direction.at(2) = t3 - p3;
    direction.at(3) = 1;

    mesh = std::make_shared<Mesh>(gsl_lite::not_null{std::move(_mat)},
                                  "models/cone.obj");
}

void SpotLight::Render(const Camera& cam) {
    const glm::vec3 position = GetPosition();
    const glm::vec3 dir(target.at(0) - position.x, target.at(1) - position.y,
                        target.at(2) - position.z);
    const float distance = glm::length(dir);
    if (distance <= 1e-6F) {
        return;
    }

    direction.at(0) = dir.x;
    direction.at(1) = dir.y;
    direction.at(2) = dir.z;
    direction.at(3) = 1.0F;

    // The cone mesh points along +Y. Align its axis with the target direction.
    const glm::vec3 unit = dir / distance;
    const float x = glm::degrees(glm::asin(glm::clamp(unit.z, -1.0F, 1.0F)));
    const float z = glm::abs(unit.z) >= 1.0F - 1e-6F
                        ? 0.0F
                        : glm::degrees(glm::atan(-unit.x, unit.y));
    mesh->SetRotation(x, 0.0F, z);

    // Cutoff is the cone's half angle; its radius is distance * tan(cutoff).
    const float radius = distance * glm::tan(glm::radians(Cutoff));
    mesh->SetScale(radius, distance, radius);
    mesh->SetPosition(position);
    mesh->Render(cam);
}

void SpotLight::SetTarget(glm::vec3 _target) {
    target.at(0) = _target.x;
    target.at(1) = _target.y;
    target.at(2) = _target.z;
    const glm::vec3 position = GetPosition();
    direction.at(0) = target.at(0) - position.x;
    direction.at(1) = target.at(1) - position.y;
    direction.at(2) = target.at(2) - position.z;
    direction.at(3) = 1;
}

SpotLight::~SpotLight() = default;

void SpotLight::SetPos(glm::vec3 pos) {
    SetPosition(pos);

    direction.at(0) = target.at(0) - pos.x;
    direction.at(1) = target.at(1) - pos.y;
    direction.at(2) = target.at(2) - pos.z;
    direction.at(3) = 1;
}

PointLight::PointLight(float d1, float d2, float d3, float r, float g, float b,
                       float p, shared_ptr<Material> _mat)
    : power(p), radius(CalculateSphereSize(r, g, b, p) / 2.0F) {
    SetPosition(d1, d2, d3);
    color.at(0) = r;
    color.at(1) = g;
    color.at(2) = b;
    temp.at(0) = 0;
    temp.at(1) = 0;
    temp.at(2) = 0;
    /*char* vertexShaderSorceCode=ReadFile("shaders/lightVS.vsh");
    char* fragmentShaderSourceCode=ReadFile("shaders/lightFS.fsh");
    shaderProgram = std::make_unique<Shader>();
    shaderProgram->AddShader(vertexShaderSorceCode,VertexShader);
    shaderProgram->AddShader(fragmentShaderSourceCode,FragmnetShader);
    shaderProgram->Link();

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(temp),NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(temp),temp);
    positionID=	    shaderProgram->GetAttribLocation("position");
    gWorldID=	    shaderProgram->GetUniformLocation("gWVCP");
    PixelColorID=   shaderProgram->GetUniformLocation("PixelColor");
    PointSizeID=    shaderProgram->GetUniformLocation("size");*/

    sphere = std::make_shared<Mesh>(gsl_lite::not_null{std::move(_mat)},
                                    "models/normal_geosphere.obj");

    spdlog::info("Light radius is {}", radius);
}
PointLight::~PointLight() = default;
void PointLight::Render(const Camera& cam) {
    sphere->SetScale(radius, radius, radius);
    sphere->SetPosition(GetPosition());
    sphere->Render(cam);
}

void PointLight::SetPos(glm::vec3 pos) { SetPosition(pos); }

void PointLight::SetCol(glm::vec3 col) {
    color.at(0) = col.x;
    color.at(1) = col.y;
    color.at(2) = col.z;

    radius = CalcSphereSize();
}
float PointLight::CalcSphereSize() {
    return CalculateSphereSize(color.at(0), color.at(1), color.at(2), power);
}
