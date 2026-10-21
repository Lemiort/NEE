#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include <fstream>

#include "Assistant.hpp"
#include "Camera.hpp"
#include "Material.hpp"
#include "MaterialObject.hpp"
#include <glm/glm.hpp>
#include "PlaceableObject.hpp"
#include "RotatableObject.hpp"
#include "ScaleableObject.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
// #include "Light.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))
using namespace std;

struct Vertex {
    glm::vec3 Pos;
    glm::vec3 UV;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    Vertex() {}
    Vertex(glm::vec3& pos, glm::vec3& uv, glm::vec3& normal, glm::vec3& tangent) {
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
    GLuint positionID, uvID, normalID, tangentID;
    GLuint gWorldID, gCamViewID;
    GLuint rotateID;
    GLuint dirLightDirID, dirLightColID;
    GLuint pointLightColID, pointLightIntID, pointLightPosID;
    GLuint camtransID, camPosID;
    GLuint specBufferID, specSamplerID;
    GLuint colSamplerUI, normSamplerUI;
    GLuint shadowMap;
    int spfaces, spverts;
    int Scale;

    // rotation angle around the vector
    float rPhi;
    // rotation vector
    glm::vec3 rv;

public:
    Mesh();
    virtual ~Mesh();
    int GetNumFaces() const;
    int GetNumVerts() const;
    void SetMaterial(shared_ptr<Material> _mat);
    bool Init(shared_ptr<Material> _mat, const char* model);
    void SetTexture(GLuint textureUnit);
    void Render(const Camera& cam);
    void SetVectorRotate(glm::vec3 v, float phi);
    void Rotate(float x, float y, float z);

private:
};
#endif  // MESH_H_INCLUDED
