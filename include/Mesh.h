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

class Mesh
{
public:
    //	GLuint shaderProgramID;
protected:
    Shader* shaderProgram;
    GLuint VBO; //vertex buffer, хранит вершины для отрисовки
    GLuint positionID,uvID,normalID,tangentID;
    GLuint IBO; //index buffer
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
    Material* mat;
    int spfaces, spverts;
    int Scale;
    //	DirectionalLight* light1;
    //	PointLight* light2;
    float position[3];
    float rotation[3];
    float scale[3];
    void SetShader(Shader* shader);

    //угол вращения вокруг вектора
    float rPhi;
    //вектор вращения
    Vector3f rv;
public:
    Mesh()
    {
        mat=NULL;
        shadowMap=0;
        rPhi = 0;
    }
    ~Mesh()
    {
        glDeleteBuffers(1,&VBO);
        glDeleteBuffers(1,&IBO);
    }
    int GetNumFaces();
    int GetNumVerts();
    void Init(GLuint shader,const char* model);
    bool Init(Material* _mat,const char* model);
    void SetTexture(GLuint textureUnit);

    //void Init();
    void Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam);
    void SetVectorRotate(Vector3f v, float phi);
    void SetRotate(float x, float y, float z);
    void SetScale(float x, float y, float z);
    void SetPosition(float x, float y, float z);
    void SetMaterial(Material* _mat);
    void Rotate(float x, float y, float z);
};
#endif // MESH_H_INCLUDED
