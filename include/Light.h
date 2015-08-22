#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include "Assistant.h"
#include "Camera.h"
#include "Shader.h"
#include "ShaderFunctions.h"
#include "Mesh.h"
#define BUFFER_OFFSET(i) ((char*)NULL +(i))

class Light
{
public:
    GLfloat	color[3];
};

class DirectionalLight: public Light
	{
    protected:
	    Shader* shaderProgram;
        Mesh* mesh;
	public:
		GLfloat direction[4];
		//GLfloat	color[3];
		DirectionalLight();
        Vector3f GetDir();
		Vector3f GetCol();
		void SetDir(Vector3f dir);
		void SetCol(Vector3f col);
		DirectionalLight(GLfloat d1,GLfloat d2,GLfloat d3,GLfloat r,GLfloat g,GLfloat b, Material* _mat);
		virtual void Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam);
		~DirectionalLight();
	};
class PointLight: public Light
	{
	public:
		float position[3];
		//float	color[3];
		float power;
		PointLight(float d1,float d2,float d3,float r,float g,float b, float p, Material* _mat);
		~PointLight();
		void Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam);
		void SetPos(Vector3f pos);
		void SetCol(Vector3f col);
	protected:
		//GLuint shaderProgramID;
		float CalcSphereSize();
		Shader* shaderProgram;
		GLuint VBO;
		GLuint positionID;
		GLuint gWorldID;
		GLuint PixelColorID;
		GLuint PointSizeID;
		float temp[3];
		float radius;
		Mesh* sphere;
	};
class SpotLight:public DirectionalLight
	{
	public:
		float Cutoff; //���� ��������� � ��������
		float position[3];
        float target[3];
		SpotLight();
		SpotLight(GLfloat t1,GLfloat t2,GLfloat t3,GLfloat r,GLfloat g,GLfloat b,float p1, float p2,float p3, float cutoff, Material*  _mat);
		~SpotLight();
		Vector3f GetPos();
		void SetPos(Vector3f pos);
		void SetTarget(Vector3f _target);
		virtual void Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam);
    private:
		void Init(GLfloat d1,GLfloat d2,GLfloat d3,GLfloat r,GLfloat g,GLfloat b,float p1, float p2,float p3, float cutoff);
	};

class Line
	{
	public:
		/*Vector3f p1;
		Vector3f p2;
		Vector3f c1;*/
		//GLuint shaderProgramID;
		GLuint PixelColorID, PointSizeID;
		GLuint gWorldID;
		GLuint positionID;
		GLuint VBO;
		float col[3];
		float pos[6];
		Line(Vector3f pos1, Vector3f pos2, Vector3f color);
		Line(Vector3f pos1, Vector3f pos2, Vector3f color,Shader* shader);
		~Line();
		void Render(Camera* pGameCamera, int width, int height);
		Shader* GetShader();
    private:

// TODO (mvideo#1#): ������� ����������� ��������� ������� �����
        Shader* shaderProgram;
	};


#endif  //LIGHT_H_INCLUDED
