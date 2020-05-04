#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include <fstream>
#include <iostream>

#include "Assistant.h"
#include "Camera.h"
#include "Material.h"
#include "MaterialObject.h"
#include "PlaceableObject.h"
#include "RotatableObject.h"
#include "ScaleableObject.h"
#include "Shader.h"
#include "Texture.h"
#include "math_3d.h"
//#include "Light.h"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))
using namespace std;

struct Vertex {
    glm::vec3 Pos;
    glm::vec3 UV;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    Vertex() {}
    Vertex(glm::vec3& pos, glm::vec3& uv, glm::vec3& normal,
           glm::vec3& tangent) {
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

    //угол вращения вокруг вектора
    float rPhi;
    //вектор вращения
    glm::vec3 rv;

public:
    Mesh();
    virtual ~Mesh();
    int GetNumFaces();
    int GetNumVerts();
    void SetMaterial(shared_ptr<Material> _mat);
    bool Init(shared_ptr<Material> _mat, const char* model);
    void SetTexture(GLuint textureUnit);
    void Render(Camera* cam);
    void SetVectorRotate(glm::vec3 v, float phi);
    void Rotate(float x, float y, float z);

private:
};
#endif  // MESH_H_INCLUDED
