#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include "Assistant.h"
#include "Camera.h"
//#include "ShaderFunctions.h"
#include <MaterialObject.h>
#include <PlaceableObject.h>
#include <RotatableObject.h>
#include <ScaleableObject.h>
#include <fstream>
#include <iostream>
#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "math_3d.h"
//#include "Light.h"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))
using namespace std;

struct Vertex {
    Vector3f Pos;
    Vector3f UV;
    Vector3f Normal;
    Vector3f Tangent;
    Vertex() {}
    Vertex(Vector3f& pos, Vector3f& uv, Vector3f& normal, Vector3f& tangent) {
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
    Vector3f rv;

public:
    Mesh();
    virtual ~Mesh();
    int GetNumFaces();
    int GetNumVerts();
    void SetMaterial(shared_ptr<Material> _mat);
    bool Init(shared_ptr<Material> _mat, const char* model);
    void SetTexture(GLuint textureUnit);
    void Render(Camera* cam);
    void SetVectorRotate(Vector3f v, float phi);
    void Rotate(float x, float y, float z);

private:
};
#endif  // MESH_H_INCLUDED
