#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include "Assistant.h"
#include "Camera.h"
//#include "ShaderFunctions.h"
#include "Shader.h"
#include "math_3d.h"
#include "Texture.h"
#include "Material.h"
#include <fstream>
#include <iostream>
#include <PlaceableObject.h>
#include <MaterialObject.h>
#include <RotatableObject.h>
#include <ScaleableObject.h>
//#include "Light.h"
#define BUFFER_OFFSET(i) ((char*)NULL +(i))
using namespace std;

struct Vertex
{
    Vector3f Pos;
    Vector3f UV;
    Vector3f Normal;
    Vector3f Tangent;
    Vertex() {}
    Vertex(Vector3f& pos, Vector3f& uv, Vector3f& normal, Vector3f& tangent)
    {
        Pos=pos;
        UV=uv;
        Normal=normal;
        Tangent=tangent;
    }
};

class Mesh: public PlaceableObject,
            public MaterialObject,
            public RotatableObject,
            public ScaleableObject
{
public:
    //	GLuint shaderProgramID;
protected:
    //Shader* shaderProgram;
    GLuint positionID,uvID,normalID,tangentID;
    GLuint gWorldID, gCamViewID;
    GLuint rotateID;
    GLuint dirLightDirID, dirLightColID;
    GLuint pointLightColID, pointLightIntID, pointLightPosID;
    GLuint camtransID,camPosID;
    //Texture colorMap, normalMap, specularMap;
    //GLuint colTexID, texBufferID,normBufferID,normSamplerID;
    GLuint specBufferID, specSamplerID;
    GLuint colSamplerUI, normSamplerUI;
    GLuint shadowMap;
    //Material* mat;
    int spfaces, spverts;
    int Scale;
    //	DirectionalLight* light1;
    //	PointLight* light2;
    //float position[3];

    //угол вращения вокруг вектора
    float rPhi;
    //вектор вращения
    Vector3f rv;

    virtual Shader* GetShader();
    virtual void SetShader(Shader* shader);

public:
    Mesh();
    virtual ~Mesh();
    int GetNumFaces();
    int GetNumVerts();
    void Init(GLuint shader,const char* model);
    bool Init(Material* _mat,const char* model);
    void SetTexture(GLuint textureUnit);

    //void Init();
    void Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam);
    void SetVectorRotate(Vector3f v, float phi);
    virtual void SetMaterial(Material* _mat);
    void Rotate(float x, float y, float z);
private:

};
#endif // MESH_H_INCLUDED
