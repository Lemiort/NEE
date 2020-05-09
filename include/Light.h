#ifndef INCLUDE_LIGHT_H
#define INCLUDE_LIGHT_H

#include <memory>

#include "Camera.h"
#include "MaterialObject.h"
#include "Mesh.h"
#include "PlaceableObject.h"
#include "Shader.h"
#include "ShaderFunctions.h"

class Light : public MaterialObject {
public:
    // TODO сделать его protected
    glm::vec4 color{1.0f};
};

class DirectionalLight : public Light {
protected:
    // shared_ptr<Shader> shaderProgram;
    std::shared_ptr<Mesh> mesh;

public:
    GLfloat direction[4];
    DirectionalLight();
    glm::vec3 GetDir();
    glm::vec3 GetCol();
    void SetDir(glm::vec3 dir);
    void SetCol(glm::vec3 col);
    DirectionalLight(GLfloat d1, GLfloat d2, GLfloat d3, GLfloat r, GLfloat g,
                     GLfloat b, std::shared_ptr<Material> _mat);
    void Render(const Camera& cam) override;
    ~DirectionalLight();
};
class PointLight : public Light, public PlaceableObject {
public:
    float power;
    PointLight(float d1, float d2, float d3, float r, float g, float b, float p,
               std::shared_ptr<Material> _mat);
    ~PointLight();
    void Render(const Camera& cam) override;
    void SetPos(glm::vec3 pos);
    void SetCol(glm::vec3 col);

protected:
    // GLuint shaderProgramID;
    float CalcSphereSize();
    // shared_ptr<Shader> shaderProgram;
    GLuint position_id;
    GLuint model_id;
    GLuint PixelColorID;
    GLuint PointSizeID;
    float temp[3];
    float radius;
    std::shared_ptr<Mesh> sphere;
};
class SpotLight : public DirectionalLight, public PlaceableObject {
public:
    float Cutoff;  // угол отсечения в радианах
                   // float position[3];
    float target[3];
    SpotLight();
    SpotLight(GLfloat t1, GLfloat t2, GLfloat t3, GLfloat r, GLfloat g,
              GLfloat b, float p1, float p2, float p3, float cutoff,
              std::shared_ptr<Material> _mat);
    ~SpotLight();
    glm::vec3 GetPos();
    void SetPos(glm::vec3 pos);
    void SetTarget(glm::vec3 _target);
    void Render(const Camera& cam) override;

private:
    void Init(GLfloat d1, GLfloat d2, GLfloat d3, GLfloat r, GLfloat g,
              GLfloat b, float p1, float p2, float p3, float cutoff);
};

class Line : public RenderableObject {
public:
    /*glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 c1;*/
    // GLuint shaderProgramID;
    GLuint PixelColorID, PointSizeID;
    GLuint model_id;
    GLuint position_id;
    float col[3];
    float pos[6];
    Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color);
    Line(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color,
         std::shared_ptr<Shader> shader);
    ~Line();
    // void Render(Camera* pGameCamera, int width, int height);
    void Render(const Camera& cam) override;
    std::shared_ptr<Shader> GetShader();

private:
};

#endif  // INCLUDE_LIGHT_H
