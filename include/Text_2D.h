#ifndef TEXT_2D_H_INCLUDED
#define TEXT_2D_H_INCLUDED
#include <SOIL.h>
#include <math_3d.h>
#include "Shader.h"
#include "ShaderFunctions.h"
#include <GL\glew.h>
#include <queue>
#include <cstring>
#include <map>
#define BUFFER_OFFSET(i) ((char*)NULL +(i))
using namespace std;

struct FontCharacter
{
//    X pos   Y pos   Width   Height   Xoffset  Yoffset  Orig W   Orig H
    unsigned int xpos, ypos, width, height, origW,origH;
    int xOffset, yOffset;
    FontCharacter(unsigned u1,unsigned u2,unsigned u3,unsigned u4,int i1,int i2,unsigned u5,unsigned u6)
    {
        xpos=u1; ypos=u2; width=u3; height=u4; origW=u5; origH=u6;
        xOffset=i1;
        yOffset=i2;
    }
};
struct KerningPairs
{
    unsigned int code1;
    unsigned int code2;
    KerningPairs(unsigned k1, unsigned int k2)
    {
        code1=k1;
        code2=k2;
    }
};



class Font2d
	{
private:
    Shader* shaderProgram;
    GLuint VBO,IBO;
    GLuint sverticesID,spositionID;
    GLuint uvID,suvID;
    GLuint sizeID;
    GLuint texSamplerID,texBufferID;
    GLuint colorID;
    Vector4f color;
    string filename;
    unsigned int* indicies;
    float aratio;
    //��������� �������� �������� � ������
    float kx,ky;
    //������ ������� � uv
    float pkx,pky;
    int fontHeight;
    uint32_t imageWidth, imageHeight;
    string fontName;
    map<unsigned int,FontCharacter> fontInfo;
protected:
    map<uint32_t,float> kerningInfo;
public:
    Font2d();
    ~Font2d();
	bool Init(string filename,Shader* _sh);
	int GetFontHeight();
	float GetWidth(unsigned int c);
	float GetHeight(unsigned int c);
	float GetSpaceWidth();
	void SetAspectRatio(int width,int height);
	void SetAspectRatio(float);
	Vector2f Render(unsigned int c, float x, float y, float size);//���������� ����� �������
	friend class FontLine2d;
	};

class FontLine2d
{
    private:
        Font2d character;
        Shader* shaderProgram;
        float prevX,prevY;
        float aratio;
        float spaceWidth;
        unsigned int prevChar;
    public:
        FontLine2d();
        ~FontLine2d();
        bool Init(string filename,Shader* _sh);
        void SetAspectRatio(int w, int h);
        void Render(string text,float x, float y, float size);
};
class Text2d
	{
	private:
	    bool yourselfShader;
    //GLuint shaderProgramID;
    Shader* shaderProgram;
    GLuint VBO,IBO;
    GLuint positionID,spositionID;
    GLuint uvID,suvID;
    GLuint sizeID;
    GLuint texSamplerID,texBufferID;
    GLuint colorID;
    Vector4f color;
    unsigned int* indicies;
    float aratio;
public:
	Text2d();
	~Text2d();
	void Init(int width,int height, Shader* _sh=NULL);
	void Init(Shader* shader, GLuint texture,GLuint texbuf);
	void SetAspectRatio(int width,int height);
	void SetAspectRatio(float);
	void Render(unsigned int c, float x, float y, float size);
	};

class TextLine2d
	{
	private:
		Text2d* symbol;
		float aratio;
		float pixelSize;
		Shader* shaderProgram;
	public:
		TextLine2d();
		~TextLine2d();
		void Init(int width, int height, Shader* _sh=NULL);
		void SetAspectRatio(int width, int height);
		void Render(float x, float y,float size, char* input);
	};
#endif //TEXT_2D_H_INCLUDED
