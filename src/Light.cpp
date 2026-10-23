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

const glm::vec3& Light::GetColor() const noexcept { return color_; }

void Light::SetColorValue(glm::vec3 value) noexcept { color_ = value; }

DirectionalLight::DirectionalLight(float d1, float d2, float d3, float r,
                                   float g, float b)
    : direction_{d1, d2, d3} {
    SetColorValue({r, g, b});
}

const glm::vec3& DirectionalLight::GetDirection() const noexcept {
    return direction_;
}

void DirectionalLight::SetDirection(glm::vec3 value) noexcept {
    direction_ = value;
}

void DirectionalLight::SetColor(glm::vec3 value) noexcept {
    SetColorValue(value);
}

PointLight::PointLight(float x, float y, float z, float r, float g, float b,
                       float lightPower)
    : radius_(CalculateLightRadius(r, g, b, lightPower)), power_(lightPower) {
    SetPosition(x, y, z);
    SetColorValue({r, g, b});
}

void PointLight::SetPos(glm::vec3 position) { SetPosition(position); }

void PointLight::SetColor(glm::vec3 value) noexcept {
    SetColorValue(value);
    radius_ = CalculateRadius();
}

float PointLight::GetRadius() const noexcept { return radius_; }

float PointLight::GetPower() const noexcept { return power_; }

float PointLight::CalculateRadius() const {
    const auto& color = GetColor();
    return CalculateLightRadius(color.r, color.g, color.b, power_);
}

SpotLight::SpotLight(float targetX, float targetY, float targetZ, float r,
                     float g, float b, float positionX, float positionY,
                     float positionZ, float cutoff)
    : DirectionalLight(targetX - positionX, targetY - positionY,
                       targetZ - positionZ, r, g, b),
      cutoff_(cutoff),
      target_(targetX, targetY, targetZ) {
    SetPosition(positionX, positionY, positionZ);
}

glm::vec3 SpotLight::GetPos() const { return GetPosition(); }

glm::vec3 SpotLight::GetDirection() const {
    return GetTarget() - GetPosition();
}

glm::vec3 SpotLight::GetTarget() const { return target_; }

float SpotLight::GetCutoff() const noexcept { return cutoff_; }

void SpotLight::SetPos(glm::vec3 position) {
    SetPosition(position);
    SetDirection(GetDirection());
}

void SpotLight::SetTarget(glm::vec3 targetPosition) {
    target_ = targetPosition;
    SetDirection(GetDirection());
}
