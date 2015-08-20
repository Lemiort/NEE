#include "skybox.h"

SkyBox::SkyBox(Shader* shader)
{
    shaderProgram=shader;
    shaderProgramID=shader->shaderProgramID;
}
SkyBox::~SkyBox()
	{

	}
bool SkyBox::Init(const string& Directory,
                  const string& PosXFilename,
                  const string& NegXFilename,
                  const string& PosYFilename,
                  const string& NegYFilename,
                  const string& PosZFilename,
                  const string& NegZFilename)
	{
	Scale=0;
	int* spindices=NULL;
	float* spvertices=NULL;
	float* spnormals=NULL; float* spuvs=NULL;
	float* sptangent=NULL;
	 spfaces=0;spverts=0;
	 try
	 {
        /*FILE* fp;
		fp=fopen("Models/sphere.ho3d","r+b");
		if(!fp) return false;
		fread(&spverts,sizeof(int),1,fp);
		spvertices=new float[spverts*3];
		spuvs=new float[spverts*2];
		spnormals=new float[spverts*3];
		for(int i=0;i<spverts;i++)
			{
			fread(&spvertices[3*i],sizeof(float),1,fp);
			fread(&spvertices[3*i+1],sizeof(float),1,fp);
			fread(&spvertices[3*i+2],sizeof(float),1,fp);
			fread(&spuvs[2*i],sizeof(float),1,fp);
			fread(&spuvs[2*i+1],sizeof(float),1,fp);
			fread(&spnormals[3*i],sizeof(float),1,fp);
			fread(&spnormals[3*i+1],sizeof(float),1,fp);
			fread(&spnormals[3*i+2],sizeof(float),1,fp);
			}
		fread(&spfaces,sizeof(int),1,fp);
		spindices=new int[spfaces*3];
		for(int i=0;i<spfaces;i++)
			{
				fread(&spindices[3*i],sizeof(int),1,fp);
				fread(&spindices[3*i+1],sizeof(int),1,fp);
				fread(&spindices[3*i+2],sizeof(int),1,fp);
			}
		fclose(fp);*/
		FILE* fp;
        fp=fopen("Models/sphere.ho3d","r+b");
        if(!fp) return false;
        fread(&spverts,sizeof(int),1,fp);
        spvertices=new float[spverts*3];
        spuvs=new float[spverts*2];
        spnormals=new float[spverts*3];
        sptangent =new float;
        for(int i=0; i<spverts; i++)
            {
                fread(&spvertices[3*i],sizeof(float),1,fp);
                fread(&spvertices[3*i+1],sizeof(float),1,fp);
                fread(&spvertices[3*i+2],sizeof(float),1,fp);
                fread(&spuvs[2*i],sizeof(float),1,fp);
                fread(&spuvs[2*i+1],sizeof(float),1,fp);
                fread(&spnormals[3*i],sizeof(float),1,fp);
                fread(&spnormals[3*i+1],sizeof(float),1,fp);
                fread(&spnormals[3*i+2],sizeof(float),1,fp);
            }
        for(int i=0; i<spverts*3; i++)
            {
                fread(sptangent,sizeof(float),1,fp);
            }
        fread(&spfaces,sizeof(int),1,fp);
        spindices=new int[spfaces*3];
        for(int i=0; i<spfaces; i++)
            {
                fread(&spindices[3*i],sizeof(int),1,fp);
                fread(&spindices[3*i+1],sizeof(int),1,fp);
                fread(&spindices[3*i+2],sizeof(int),1,fp);
            }
        fclose(fp);
	 }
    catch(const std::bad_alloc&)
    {
        printf("\nError creating new Mesh in Skybox");
        return false;
    }
	//������ ������, � ������� ����� ������� ��
	glGenBuffers(1,&VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	//������ ������
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*3*(spverts),NULL,GL_STATIC_DRAW);
	//��������� ������� � ������
	glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(float)*3*spverts,spvertices);

	//����������� ������� � �������
	glGenBuffers(1,&IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(int)*3*spfaces,spindices,GL_STATIC_DRAW);

    try{
	pCubemapTex=new CubemapTexture(Directory,
                                       PosXFilename,
                                       NegXFilename,
                                       PosYFilename,
                                       NegYFilename,
                                       PosZFilename,
                                       NegZFilename);
	}
	catch(const std::bad_alloc&)
    {
        printf("\nError creating new Cubemap");
        return false;
    }
	pCubemapTex->Load();

	positionID=	shaderProgram->GetAttribLocation("s_vPosition");
	//������� ������� uniform-����������
	gWorldID=	shaderProgram->GetUniformLocation("gWVP");

	//������ �� �� �����
	pCubemapTex->Bind(GL_TEXTURE2);
	textureID= shaderProgram->GetUniformLocation("gCubemapTexture");
	glActiveTexture(GL_TEXTURE2);
	glUniform1i(textureID,2);
	for(int i=0; i<3;i++)
		{
		position[i]=0;
		rotation[i]=0;
		scale[i]=1;
		}

    //������ ������
    delete[] spindices;
    delete[] spvertices;
    delete[] spnormals;
    delete[] spuvs;
    delete sptangent;
	return true;
	}
void SkyBox::Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam)
	{
	 GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);

	Assistant TM;//TM - ��� �������, 2- ��� ������� �������, 3 - ��� ������� ������ ��� ���������
	 TM.Scale(3,3,3);
    TM.WorldPos(cam->GetPos().x,cam->GetPos().y,cam->GetPos().z);
	TM.Rotate(180,180,0);
	TM.SetCamera(cam->GetPos(), cam->GetTarget(), cam->GetUp());
	TM.SetPerspectiveProj(FOV, Width,Height, zNear, zFar);

	//glUseProgram(shaderProgramID);
	shaderProgram->Use();
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(positionID,3,GL_FLOAT,GL_FALSE,0,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO);


	pCubemapTex->Bind(GL_TEXTURE2);
	glUniform1i(textureID,2);
	//cout<<colTexID<<"\n"<<textureID<<"\n";

    glUniformMatrix4fv(gWorldID, 1, GL_TRUE, (const GLfloat*)TM.GetTSRVC());


	glEnableVertexAttribArray(positionID);
	glDrawElements(GL_TRIANGLES,spfaces*3,GL_UNSIGNED_INT,NULL);
	glDisableVertexAttribArray(positionID);

	glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
	}
