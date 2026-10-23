#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED

#include <array>
#include <glm/ext/vector_float3.hpp>

#include "Transformable.hpp"

class Light {
public:
    std::array<float, 3> color{};
};

class DirectionalLight : public Light {
public:
    DirectionalLight() = default;
    DirectionalLight(float d1, float d2, float d3, float r, float g, float b);

    [[nodiscard]] glm::vec3 GetDir() const;
    [[nodiscard]] glm::vec3 GetCol() const;
    void SetDir(glm::vec3 direction);
    void SetCol(glm::vec3 color);

    std::array<float, 4> direction{};
};

class PointLight : public Light, public Transformable {
public:
    PointLight(float x, float y, float z, float r, float g, float b,
               float power);
    PointLight(PointLight&&) noexcept = default;
    PointLight& operator=(PointLight&&) noexcept = default;

    void SetPos(glm::vec3 position);
    void SetCol(glm::vec3 color);
    [[nodiscard]] float GetRadius() const noexcept;

    float power{};

private:
    [[nodiscard]] float CalculateRadius() const;

    float radius{};
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
    void SetPos(glm::vec3 position);
    void SetTarget(glm::vec3 targetPosition);

    float Cutoff{};
    std::array<float, 3> target{};
};

#endif  // LIGHT_H_INCLUDED
