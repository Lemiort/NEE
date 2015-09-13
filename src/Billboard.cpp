#include "Billboard.h"

Billboard::Billboard()
{
    Pos=Vector3f(1,1,1);
    shader=false;
}

/*Billboard::Billboard(GLuint _shader)
{
    Pos=Vector3f(0,0,0);
    shaderProgramID=_shader;
    shader=true;
}*/

Billboard::Billboard(Shader* _shader)
{
    Pos=Vector3f(0,0,0);
    shaderProgram=_shader;
   // shaderProgramID=_shader->shaderProgramID;
    shader=true;
}


Billboard::~Billboard()
{
    //dtor
}

void Billboard::Init(const char* TexFilename)
{
    if(shader==false)
    {
        char* vertexShaderSorceCode=ReadFile("Shaders/billboard.vsh");
        char* fragmentShaderSourceCode=ReadFile("Shaders/billboard.fsh");
        char* geometryShaderSourceCode=ReadFile("Shaders/billboard.gsh");
        shaderProgram=new Shader;
        shaderProgram->AddShader(vertexShaderSorceCode, VertexShader);
        shaderProgram->AddShader(fragmentShaderSourceCode,FragmnetShader);
        shaderProgram->AddShader(geometryShaderSourceCode,GeometryShader);
        shaderProgram->Init();
        /*GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
        GLuint fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode);
        GLuint geometryShaderID=MakeGeometryShader(geometryShaderSourceCode);
        shaderProgramID=MakeShaderProgram(vertexShaderID,geometryShaderID, fragmentShaderID);*/
       //shaderProgramID=MakeShaderProgram(vertexShaderID, fragmentShaderID);
        delete[] vertexShaderSorceCode;
        delete[] fragmentShaderSourceCode;
        delete[] geometryShaderSourceCode;
        shader=true;
    }
    colorMap.Load(TexFilename);
    colorMap.Bind(GL_TEXTURE0);
    colSamplerID =   shaderProgram->GetUniformLocation("colTexSampler");
	glUniform1i(colSamplerID, 0);


	glGenBuffers(1,&VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	//создаЄм буффер
		float coords[3]={Pos.x,Pos.y,Pos.z};
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*(3),coords,GL_DYNAMIC_DRAW);
    positionID=     shaderProgram->GetAttribLocation("s_vPosition");
    camViewID=	shaderProgram->GetUniformLocation("gVP");
    camPosID=     shaderProgram->GetUniformLocation("gCameraPos");
}
void Billboard::SetPos(Vector3f _Pos)
{
    Pos=_Pos;
   // glUseProgram(shaderProgramID);
   shaderProgram->Use();
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
	//создаЄм буффер
		float coords[3]={Pos.x,Pos.y,Pos.z};
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*(3),coords,GL_DYNAMIC_DRAW);
    positionID=shaderProgram->GetAttribLocation("s_vPosition");
}

void Billboard::Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam)
{
    //glUseProgram(shaderProgramID);
    shaderProgram->Use();
    Assistant TM;//TM - ƒл€ объекта, 2- дл€ нормали объекта, 3 - дл€ позиции камера дл€ спекул€ра
    TM.SetCamera(cam->GetPos(), cam->GetTarget(), cam->GetUp());
    TM.SetPerspectiveProj(30.0f, Width, Height, 1.0f, 1000.0f);

    //матрица проекции камеры
    glUniformMatrix4fv(camViewID, 1, GL_TRUE, (const GLfloat*)TM.GetVC());
    //позици€ камеры
    glUniform3f(camPosID,cam->GetPos().x,cam->GetPos().y,cam->GetPos().z);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(positionID,3,GL_FLOAT,GL_FALSE,0,nullptr);

	colorMap.Bind(GL_TEXTURE0);
	glUniform1i(colSamplerID,0);//говорим шейдеру, чтобы использовал в качестве текстуры 0

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(positionID,3,GL_FLOAT,GL_FALSE,0,nullptr);
	glEnableVertexAttribArray(positionID);
	glDrawArrays(GL_POINTS, 0, 1);
	glDisableVertexAttribArray(positionID);
}
