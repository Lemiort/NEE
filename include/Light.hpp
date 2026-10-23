#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED

#include <glm/ext/vector_float3.hpp>

#include "Transformable.hpp"

class Light {
public:
    [[nodiscard]] const glm::vec3& GetColor() const noexcept;

protected:
    void SetColorValue(glm::vec3 color) noexcept;

private:
    glm::vec3 color_{0.0F};
};

class DirectionalLight : public Light {
public:
    DirectionalLight() = default;
    DirectionalLight(float d1, float d2, float d3, float r, float g, float b);

    [[nodiscard]] const glm::vec3& GetDirection() const noexcept;
    void SetDirection(glm::vec3 direction) noexcept;
    void SetColor(glm::vec3 color) noexcept;

private:
    glm::vec3 direction_{0.0F};
};

class PointLight : public Light, public Transformable {
public:
    PointLight(float x, float y, float z, float r, float g, float b,
               float power);
    PointLight(PointLight&&) noexcept = default;
    PointLight& operator=(PointLight&&) noexcept = default;

    void SetPos(glm::vec3 position);
    void SetColor(glm::vec3 color) noexcept;
    [[nodiscard]] float GetRadius() const noexcept;
    [[nodiscard]] float GetPower() const noexcept;

private:
    [[nodiscard]] float CalculateRadius() const;

    float radius_{};
    float power_{};
};

class SpotLight : public DirectionalLight, public Transformable {
public:
    SpotLight() = default;
    SpotLight(float targetX, float targetY, float targetZ, float r, float g,
              float b, float positionX, float positionY, float positionZ,
              float cutoff);
    SpotLight(SpotLight&&) noexcept = default;
    SpotLight& operator=(SpotLight&&) noexcept = default;

    [[nodiscard]] glm::vec3 GetPos() const;
    [[nodiscard]] glm::vec3 GetDirection() const;
    [[nodiscard]] glm::vec3 GetTarget() const;
    [[nodiscard]] float GetCutoff() const noexcept;
    void SetPos(glm::vec3 position);
    void SetTarget(glm::vec3 targetPosition);

private:
    float cutoff_{};
    glm::vec3 target_{0.0F};
};

#endif  // LIGHT_H_INCLUDED
