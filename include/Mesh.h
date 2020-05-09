#ifndef INCLUDE_MESH_H
#define INCLUDE_MESH_H

#include <glm/glm.hpp>
#include <memory>

#include "Camera.h"
#include "Material.h"
#include "MaterialObject.h"
#include "PlaceableObject.h"
#include "RotatableObject.h"
#include "ScaleableObject.h"
#include "Shader.h"
#include "Texture.h"

struct Vertex {
    glm::vec3 Pos;
    glm::vec3 UV;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    Vertex() {}
    Vertex(const glm::vec3& pos, const glm::vec3& uv, const glm::vec3& normal,
           const glm::vec3& tangent) {
        Pos = pos;
        UV = uv;
        Normal = normal;
        Tangent = tangent;
    }
};

class Mesh : public PlaceableObject,
             public MaterialObject,
             public RotatableObject,
             public ScaleableObject {
public:
protected:
    GLuint position_id, uv_id, normal_id, tangent_id;
    GLuint model_id, view_projection_id;
    GLuint rotation_id;
    GLuint dirLightDirID, dirLightColID;
    GLuint pointLightColID, pointLightIntID, pointLightPosID;
    GLuint camtransID, camPosID;
    GLuint specBufferID, specSamplerID;
    GLuint colSamplerUI, normSamplerUI;
    GLuint shadowMap;
    int faces_count, vertices_count;
    int Scale;

    float rotation_angle;
    glm::vec3 rotation_vector;

public:
    Mesh();
    virtual ~Mesh();
    int GetFacesCount();
    int GetVerticesCount();
    void SetMaterial(std::shared_ptr<Material> _mat);
    bool Init(std::shared_ptr<Material> _mat, const char* model);
    void SetTexture(GLuint textureUnit);
    void Render(const Camera& cam) override;
    void SetVectorRotate(glm::vec3 v, float phi);
    void Rotate(float x, float y, float z);

private:
};

#endif  // INCLUDE_MESH_H
