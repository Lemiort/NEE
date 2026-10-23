#include "Light.hpp"

#include <cmath>
#include <glm/geometric.hpp>

namespace {
float CalculateLightRadius(float red, float green, float blue, float power) {
    return ((8.0F *
             std::sqrt(glm::length(glm::vec3(red, green, blue)) * power)) +
            1.0F) /
           2.0F;
}
}  // namespace

DirectionalLight::DirectionalLight(float d1, float d2, float d3, float r,
                                   float g, float b)
    : direction{d1, d2, d3, 1.0F} {
    color = {r, g, b};
}

glm::vec3 DirectionalLight::GetDir() const {
    return {direction.at(0), direction.at(1), direction.at(2)};
}

glm::vec3 DirectionalLight::GetCol() const {
    return {color.at(0), color.at(1), color.at(2)};
}

void DirectionalLight::SetDir(glm::vec3 value) {
    direction = {value.x, value.y, value.z, 1.0F};
}

void DirectionalLight::SetCol(glm::vec3 value) {
    color = {value.x, value.y, value.z};
}

PointLight::PointLight(float x, float y, float z, float r, float g, float b,
                       float lightPower)
    : power(lightPower), radius(CalculateLightRadius(r, g, b, lightPower)) {
    SetPosition(x, y, z);
    color = {r, g, b};
}

void PointLight::SetPos(glm::vec3 position) { SetPosition(position); }

void PointLight::SetCol(glm::vec3 value) {
    color = {value.x, value.y, value.z};
    radius = CalculateRadius();
}

float PointLight::GetRadius() const noexcept { return radius; }

float PointLight::CalculateRadius() const {
    return CalculateLightRadius(color.at(0), color.at(1), color.at(2), power);
}

SpotLight::SpotLight(float targetX, float targetY, float targetZ, float r,
                     float g, float b, float positionX, float positionY,
                     float positionZ, float cutoff)
    : DirectionalLight(targetX - positionX, targetY - positionY,
                       targetZ - positionZ, r, g, b),
      Cutoff(cutoff),
      target{targetX, targetY, targetZ} {
    SetPosition(positionX, positionY, positionZ);
}

glm::vec3 SpotLight::GetPos() const { return GetPosition(); }

glm::vec3 SpotLight::GetDirection() const {
    return glm::vec3(target.at(0), target.at(1), target.at(2)) - GetPosition();
}

void SpotLight::SetPos(glm::vec3 position) {
    SetPosition(position);
    SetDir(GetDirection());
}

void SpotLight::SetTarget(glm::vec3 targetPosition) {
    target = {targetPosition.x, targetPosition.y, targetPosition.z};
    SetDir(GetDirection());
}
