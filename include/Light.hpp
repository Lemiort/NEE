#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <memory>

#include "Camera.hpp"
#include "Material.hpp"
#include "MaterialObject.hpp"
#include "Mesh.hpp"
#include "Transformable.hpp"

class Light : public MaterialObject {
public:
    Light() = default;
    Light(Light&&) noexcept = default;
    Light& operator=(Light&&) noexcept = default;
    // TODO make it protected
    std::array<GLfloat, 3> color{};
};

class DirectionalLight : public Light {
protected:
    // shared_ptr<Shader> shaderProgram;
    shared_ptr<Mesh> mesh;

public:
    std::array<GLfloat, 4> direction{};
    // GLfloat	color[3];
    DirectionalLight();
    DirectionalLight(DirectionalLight&&) noexcept = default;
    DirectionalLight& operator=(DirectionalLight&&) noexcept = default;
    glm::vec3 GetDir();
    glm::vec3 GetCol();
    void SetDir(glm::vec3 dir);
    void SetCol(glm::vec3 col);
    DirectionalLight(GLfloat d1, GLfloat d2, GLfloat d3, GLfloat r, GLfloat g,
                     GLfloat b, shared_ptr<Material> _mat);
    void Render(const Camera& cam) override;
    void DrawVolume(const Material& material,
                    const MeshShaderBindings& bindings) const;
    ~DirectionalLight() override;
};
class PointLight : public Light, public Transformable {
public:
    // float position[3];
    // float	color[3];
    float power;
    PointLight(float d1, float d2, float d3, float r, float g, float b, float p,
               shared_ptr<Material> _mat);
    PointLight(PointLight&&) noexcept = default;
    PointLight& operator=(PointLight&&) noexcept = default;
    ~PointLight() override;
    void Render(const Camera& cam) override;
    void DrawVolume(const Material& material,
                    const MeshShaderBindings& bindings) const;
    void SetPos(glm::vec3 pos);
    void SetCol(glm::vec3 col);

protected:
    // GLuint shaderProgramID;
    float CalcSphereSize();
    // shared_ptr<Shader> shaderProgram;
    GLint positionID{};
    GLint gWorldID{};
    GLint PixelColorID{};
    GLint PointSizeID{};
    std::array<float, 3> temp{};
    float radius{};
    shared_ptr<Mesh> sphere;
};
class SpotLight : public DirectionalLight, public Transformable {
public:
    float Cutoff{};  // cone half angle in degrees
                     //  float position[3];
    std::array<float, 3> target{};
    SpotLight();
    SpotLight(SpotLight&&) noexcept = default;
    SpotLight& operator=(SpotLight&&) noexcept = default;
    SpotLight(GLfloat t1, GLfloat t2, GLfloat t3, GLfloat r, GLfloat g,
              GLfloat b, float p1, float p2, float p3, float cutoff,
              shared_ptr<Material> _mat);
    ~SpotLight() override;
    glm::vec3 GetPos();
    [[nodiscard]] glm::vec3 GetDirection() const;
    void SetPos(glm::vec3 pos);
    void SetTarget(glm::vec3 _target);
    void Render(const Camera& cam) override;
    void DrawVolume(const Material& material,
                    const MeshShaderBindings& bindings) const;
};

#endif  // LIGHT_H_INCLUDED
