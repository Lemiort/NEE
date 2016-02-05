#ifndef TEXT_2D_H_INCLUDED
#define TEXT_2D_H_INCLUDED
#include <SOIL.h>
#include <math_3d.h>
#include "Shader.h"
#include "ShaderFunctions.h"
#include <GL\glew.h>
#include <RenderableObject.h>
#include <PlaceableObject.h>
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
    FontCharacter(unsigned u1 = 0,
                  unsigned u2 = 0,
                  unsigned u3 = 0,
                  unsigned u4 = 0,
                  int i1 = 0,
                  int i2 = 0,
                  unsigned u5 = 0,
                  unsigned u6 = 0)
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


//�����
class Font2d: public RenderableObject, public PlaceableObject
	{
private:
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


    float realWidth;
    float realHeight;
    float dx;
    float xOffset;
    float yOffset;
    FontCharacter temp;
protected:
    Vector2f characterLength;
    unsigned int character;
    map<uint32_t,float> kerningInfo;
public:
    Font2d();
    ~Font2d();
	bool Init(string filename,shared_ptr<Shader> _sh);
	int GetFontHeight();
	float GetWidth(unsigned int c);
	float GetHeight(unsigned int c);
	float GetSpaceWidth();
	void SetAspectRatio(int width,int height);
	void SetAspectRatio(float);
	void SetCharacter(unsigned int c);

	Vector2f GetLastCharacterLength();//���������� ����� �������� �������
	void Render(Camera* cam);
	friend class FontLine2d;
	};

//������� ������
class FontLine2d: public RenderableObject, public PlaceableObject
{
    private:
        Font2d character;
        shared_ptr<Shader> shaderProgram;
        float prevX,prevY;
        float aratio;
        float spaceWidth;
        unsigned int prevChar;
        string text;
    public:
        FontLine2d();
        ~FontLine2d();
        bool Init(string filename,shared_ptr<Shader> _sh);
        void SetAspectRatio(int w, int h);
        void SetText(string _text);
        //void Render(string text,float x, float y, float size);
        void Render(Camera* cam);
};

//����� ��������� ����������
class Text2d: public RenderableObject, public PlaceableObject
	{
	private:
	    bool yourselfShader;
    //GLuint shaderProgramID;
    //shared_ptr<Shader> shaderProgram;
    GLuint positionID,spositionID;
    GLuint uvID,suvID;
    GLuint sizeID;
    GLuint texSamplerID,texBufferID;
    GLuint colorID;
    Vector4f color;
    unsigned int* indicies;
    float aratio;

    //����� ������� � ������� �������
    unsigned int character;
public:
	Text2d();
	~Text2d();
	void Init(int width,int height, shared_ptr<Shader> _sh=NULL);
	void Init(shared_ptr<Shader> shader, GLuint texture,GLuint texbuf);
	void SetAspectRatio(int width,int height);
	void SetAspectRatio(float);
	void SetCharacter(unsigned int c);
	//void Render(unsigned int c, float x, float y, float size);
	void Render(Camera* cam);
	};

//����� ����� �� ��������� ����
class TextLine2d: public RenderableObject, public PlaceableObject
	{
	private:
		Text2d* symbol;
		float aratio;
		float pixelSize;
		//shared_ptr<Shader> shaderProgram;
		string text;
	public:
		TextLine2d();
		~TextLine2d();
		void Init(int width, int height, shared_ptr<Shader> _sh=NULL);
		void SetAspectRatio(int width, int height);
		void SetText(string _text);
		//void Render(float x, float y,float size, char* input);
		void Render(Camera* cam);
	};
#endif //TEXT_2D_H_INCLUDED
