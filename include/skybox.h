#ifndef SKYBOX_H_INCLUDED
#define SKYBOX_H_INCLUDED
#include "cubemap_texture.h"
//#include "ShaderFunctions.h"
#include "Shader.h"
#include "Camera.h"
#include "Assistant.h"
#include "tga_loader.h"
#include <RenderableObject.h>
#include <PlaceableObject.h>
#define BUFFER_OFFSET(i) ((char*)NULL +(i))
class SkyBox: public RenderableObject, public PlaceableObject
{
public:
    SkyBox(Shader* shader);
    ~SkyBox();

    bool Init(const string& Directory,
              const string& PosXFilename,
              const string& NegXFilename,
              const string& PosYFilename,
              const string& NegYFilename,
              const string& PosZFilename,
              const string& NegZFilename);

    void Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam);
	GLuint shaderProgramID;
	GLuint VBO; //vertex buffer, ������ ������� ��� ���������
		GLuint positionID;
		GLuint IBO; //index buffer
		GLuint gWorldID, gCamViewID;
		int spfaces, spverts;
		int Scale;
	void Init(GLuint shader,const char* model);
private:
    const Camera* pCamera;
    CubemapTexture* pCubemapTex;
	GLuint WVPID;
    GLuint textureID;
    //Shader* shaderProgram;
public:
		GLuint uvID,normalID;
		GLuint rotateID;
		GLuint camtransID,camPosID;
		GLuint colTexID,texBufferID;

		//float position[3];
		float rotation[3];
		float scale[3];
};

#endif // SKYBOX_H_INCLUDED
