#include "Light.h"
#include <iomanip>

void DirectionalLight::SetCol(Vector3f col)
{
    color[0] = col.x;
    color[1] = col.y;
    color[2] = col.z;
}

void DirectionalLight::SetDir(Vector3f dir)
{
    direction[0] = dir.x;
    direction[1] = dir.y;
    direction[2] = dir.z;
}

DirectionalLight::DirectionalLight(GLfloat d1,GLfloat d2,GLfloat d3,GLfloat r,GLfloat g,GLfloat b, Material* _mat)
			{
				direction[0]=d1;
				direction[1]=d2;
				direction[2]=d3;
				direction[3]=1;
				color[0]=r;
				color[1]=g;
				color[2]=b;

				mesh =  new Mesh();
				//mesh->Init(_mat,"Models/quad2x2front.ho3d");
				mesh->Init(_mat,"Models/cube2x2x2.ho3d");
			}

void DirectionalLight::Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam)
			{
				//mesh->SetScale(1.0,1.0,1.0);
				mesh->SetPosition(0,0,0);
				mesh->Render(FOV,Width,Height,zNear,zFar,cam);
			}

Vector3f DirectionalLight::GetDir()
{
    return Vector3f(direction[0],
				direction[1],
				direction[2]);
}
Vector3f DirectionalLight::GetCol()
{
    return Vector3f(color[0],
				color[1],
				color[2]);
}


DirectionalLight::DirectionalLight()
			{
			}
DirectionalLight::~DirectionalLight()
			{
			}

Vector3f SpotLight::GetPos()
{
    return Vector3f(position[0],position[1],position[2]);
}

SpotLight::SpotLight()
	{
	}

SpotLight::SpotLight(GLfloat t1,GLfloat t2,GLfloat t3,GLfloat r,GLfloat g,GLfloat b,float p1, float p2, float p3, float cut, Material* _mat)
	{
				color[0]=r;
				color[1]=g;
				color[2]=b;
				position[0]=p1;
				position[1]=p2;
				position[2]=p3;
				target[0] = t1;
				target[1] = t2;
				target[2] = t3;
				 direction[0]=t1-p1;
				direction[1]=t2-p2;
				direction[2]=t3-p3;
				direction[3]=1;
				Cutoff=cut;

		mesh =  new Mesh();
        mesh->Init(_mat,"Models/cone.ho3d");
        mesh->SetRotate(90,0,0);

	}

void SpotLight::Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam)
			{
			   /* Vector3f HTarget(direction[0], 0.0, direction[2]);
                HTarget.Normalize();

                float m_AngleH;
                float m_AngleV;

                if (HTarget.z >= 0.0f)
                {
                    if (HTarget.x >= 0.0f)
                    {
                        m_AngleH = 360.0f - ToDegree(asin(HTarget.z));
                    }
                    else
                    {
                        m_AngleH = 180.0f + ToDegree(asin(HTarget.z));
                    }
                }
                else
                {
                    if (HTarget.x >= 0.0f)
                    {
                        m_AngleH = ToDegree(asin(-HTarget.z));
                    }
                    else
                    {
                        m_AngleH = 90.0f + ToDegree(asin(-HTarget.z));
                    }
                }

                m_AngleV = -ToDegree(asin(direction[1]));


			    const Vector3f Vaxis(0.0f, 1.0f, 0.0f);

                // Rotate the view vector by the horizontal angle around the vertical axis
                //Vector3f View(1.0f, 0.0f, 0.0f);
                //View.Rotate(m_AngleH, Vaxis);
                //View.Normalize();
                mesh->SetRotate(0,0,m_AngleH);
                cout<<"\r"<<m_AngleH<<" "<<m_AngleV;*/

                /*// Rotate the view vector by the vertical angle around the horizontal axis
                Vector3f Haxis = Vaxis.Cross(View);
                Haxis.Normalize();
                View.Rotate(m_AngleV, Haxis);*/

                //======Предыдущее решение задачи========
                direction[0]=target[0] - position[0];
                direction[1]=target[1] - position[1];
                direction[2]=target[2] - position[2];
                direction[3]=1;


                Vector3f dir(direction[0],
                                     direction[1],
                                     direction[2]);
                Vector3f proj(0,1,0);
                Vector3f directionXY(direction[0],
                                     direction[1],
                                     0);
                Vector3f directionYZ(0,
                                     direction[1],
                                     direction[2]);
                Vector3f directionXZ(direction[0],
                                     0,
                                     direction[2]);

                float xrot = 0, yrot = 0, zrot = 0;

                cout<<"\rproj:("<<setprecision(1);

                //X axis
                {
                    //get the pojection
                    Vector3f projYZ(0,proj.y,proj.z);

                    //calc rotation
                    if(projYZ.Cross(directionYZ).x>0)
                        xrot = -ToDegree(CalcAngle(directionYZ,projYZ));
                    else
                        xrot = ToDegree(CalcAngle(directionYZ,projYZ));

                    //update up vector
                    proj.Rotate(-xrot,Vector3f(1,0,0));
                    cout<<proj.x<<",  "<<proj.y<<",  "<<proj.z<<");  (";
                }

                //Y axis
                if(proj != dir)
                {
                    //get the pojection
                    Vector3f projXZ(proj.x,0,proj.z);

                    //calc the rotation
                    if(projXZ.Cross(directionXZ).y>0)
                        yrot = -ToDegree(CalcAngle(directionXZ,projXZ));
                    else
                        yrot = ToDegree(CalcAngle(directionXZ,projXZ));

                    //update the up vector
                    proj.Rotate(-yrot,Vector3f(0,1,0));
                    cout<<proj.x<<",  "<<proj.y<<",  "<<proj.z<<");  (";


                    //Z axis
                    if(proj != dir)
                    {
                        //get the pojection
                        Vector3f projXY(proj.x,proj.y,0);

                        //calc the rotation
                        if(projXY.Cross(directionXY).z>0)
                            zrot = ToDegree(CalcAngle(directionXY,projXY));
                        else
                            zrot = -ToDegree(CalcAngle(directionXY,projXY));
                        proj.Rotate(-zrot,Vector3f(0,0,1));
                    }
                }
                cout<<proj.x<<",  "<<proj.y<<",  "<<proj.z<<");  rot: ";




                cout<< setprecision(1)<<(int)xrot<<" "<<(int)yrot<<" "<<(int)zrot<<", dir:("<<
                        direction[0]<<", "<<direction[1]<<","<<direction[2]<<")";

               mesh->SetRotate(xrot,
                                yrot,
                                zrot);
                //=======================================================

                /*//==========решение на основе сферических координат=======
                Vector3f sphericalDirection = ToSphericalCoordinates( dir );
                Vector3f sphericalUp = ToSphericalCoordinates( Vector3f(0.0,
                                                                        1.0,
                                                                        0.0) );
                mesh->SetRotate( sphericalDirection.y-sphericalUp.y,
                                0.0f,
                                sphericalDirection.z - sphericalUp.z);
                //=========================================================*/

                //=====решение на основе поворота вокруг вектора============
                //Vector3f rotateVector =  dir.Cross(proj);
                /*mesh->SetVectorRotate(rotateVector,
                                      CalcAngle(dir,proj));*/
               // mesh->SetVectorRotate(proj,
               //                       -90.0f);
                //==========================================================

                //TODO идея: использовать скалярное произведение векторов

                mesh->SetScale(dir.Lenght()*cos(ToRadian(Cutoff)),
                               dir.Lenght(),
                               dir.Lenght()*cos(ToRadian(Cutoff)));

				mesh->SetPosition(position[0],position[1],position[2]);
				mesh->Render(FOV,Width,Height,zNear,zFar,cam);
			}

void SpotLight::SetTarget(Vector3f _target)
{
    target[0] = _target.x;
    target[1] = _target.y;
    target[2] = _target.z;
    direction[0]=target[0] - position[0];
    direction[1]=target[1] - position[1];
    direction[2]=target[2] - position[2];
    direction[3]=1;
}

SpotLight::~SpotLight()
	{
	}
void SpotLight::Init(GLfloat d1,GLfloat d2,GLfloat d3,GLfloat r,GLfloat g,GLfloat b,float p1, float p2, float p3, float cut)
	{
		direction[0]=d1;
				direction[1]=d2;
				direction[2]=d3;
				direction[3]=1;
				color[0]=r;
				color[1]=g;
				color[2]=b;
				position[0]=p1;
				position[1]=p2;
				position[2]=p3;
				Cutoff=cut;
	}

void  SpotLight::SetPos(Vector3f pos)
{
    position[0]=pos.x;
    position[1]=pos.y;
    position[2]=pos.z;

     direction[0]=target[0] - position[0];
    direction[1]=target[1] - position[1];
    direction[2]=target[2] - position[2];
    direction[3]=1;
}

PointLight::PointLight(float d1,float d2,float d3,float r,float g,float b, float p, Material* _mat)
			{
				position[0]=d1;
				position[1]=d2;
				position[2]=d3;
				color[0]=r;
				color[1]=g;
				color[2]=b;
				power=p;
				temp[0]=0; temp[1]=0; temp[2]=0;
				char* vertexShaderSorceCode=ReadFile("Shaders/lightVS.vsh");
				char* fragmentShaderSourceCode=ReadFile("Shaders/lightFS.fsh");
				shaderProgram=new Shader();
				shaderProgram->AddShader(vertexShaderSorceCode,VertexShader);
				shaderProgram->AddShader(fragmentShaderSourceCode,FragmnetShader);
				shaderProgram->Init();
				/*GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
				GLuint fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode);
				shaderProgramID=MakeShaderProgram(vertexShaderID, fragmentShaderID);*/

				glGenBuffers(1, &VBO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER,sizeof(temp),NULL, GL_STATIC_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(temp),temp);
				positionID=	    shaderProgram->GetAttribLocation("position");
				gWorldID=	    shaderProgram->GetUniformLocation("gWVCP");
				PixelColorID=   shaderProgram->GetUniformLocation("PixelColor");
				PointSizeID=    shaderProgram->GetUniformLocation("size");

				sphere =  new Mesh();
				sphere->Init(_mat,"Models/normal_sphere.ho3d");
				radius = CalcSphereSize();
				std::cout<<"\nLight radius is "<<radius;
			}
PointLight::~PointLight()
			{
			    delete shaderProgram;
			    delete sphere;
			}
void PointLight::Render(float FOV, float Width, float Height, float zNear, float zFar, Camera* cam)
			{
				Assistant TM;
				TM.WorldPos(position[0],position[1],position[2]);
				TM.SetCamera(cam->GetPos(), cam->GetTarget(), cam->GetUp());
				TM.SetPerspectiveProj(FOV, Width, Height, zNear, zFar);


                shaderProgram->Use();
				//glUseProgram(shaderProgramID);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glVertexAttribPointer(positionID,3,GL_FLOAT,GL_FALSE,0,0);


				glUniformMatrix4fv(gWorldID, 1, GL_TRUE, (const GLfloat*)TM.GetTSRVC());
				glUniform4f(PixelColorID,color[0],color[1],color[2],1);
				glUniform1f(PointSizeID, 5.0);
				glEnableVertexAttribArray(positionID);
				glDrawArrays(GL_POINTS,0,1);
				glDisableVertexAttribArray(positionID);

				sphere->SetScale(radius,radius,radius);
				sphere->SetPosition(position[0],position[1],position[2]);
				sphere->Render(FOV,Width,Height,zNear,zFar,cam);
			}

void PointLight::SetPos(Vector3f pos)
{
    position[0]=pos.x;
    position[1]=pos.y;
    position[2]=pos.z;
}

void PointLight::SetCol(Vector3f col)
{
    color[0]=col.x;
    color[1]=col.y;
    color[2]=col.z;

    radius = CalcSphereSize();
}
float PointLight::CalcSphereSize()
{
    return 8.0f*sqrtf(Vector3f(color[0],color[1],color[2]).Lenght()*power)+1.0f;
}

Line::Line(Vector3f pos1, Vector3f pos2, Vector3f color)
			{
				char* vertexShaderSorceCode=ReadFile("Shaders/lightVS.vsh");
				char* fragmentShaderSourceCode=ReadFile("Shaders/lightFS.fsh");
				shaderProgram=new Shader();
				shaderProgram->AddShader(vertexShaderSorceCode,VertexShader);
				shaderProgram->AddShader(fragmentShaderSourceCode,FragmnetShader);
				shaderProgram->Init();
				/*GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
				GLuint fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode);
				shaderProgramID=MakeShaderProgram(vertexShaderID, fragmentShaderID);*/

				glGenBuffers(1, &VBO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float),NULL, GL_STATIC_DRAW);

				pos[0]=pos1.x;
				pos[1]=pos1.y;
				pos[2]=pos1.z;
				pos[3]=pos2.x;
				pos[4]=pos2.y;
				pos[5]=pos2.z;

				col[0]=color.x;
				col[1]=color.y;
				col[2]=color.z;
				glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(pos),pos);
				positionID=	shaderProgram->GetAttribLocation("position");
				gWorldID=	shaderProgram->GetUniformLocation("gWVCP");
				PixelColorID= shaderProgram->GetUniformLocation("PixelColor");
				PointSizeID= shaderProgram->GetUniformLocation( "size");

			}
Line::Line(Vector3f pos1, Vector3f pos2, Vector3f color,Shader* shader)
			{
				//shaderProgramID=shader;
				shaderProgram=shader;
				glGenBuffers(1, &VBO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, 6*sizeof(float),NULL, GL_STATIC_DRAW);

				pos[0]=pos1.x;
				pos[1]=pos1.y;
				pos[2]=pos1.z;
				pos[3]=pos2.x;
				pos[4]=pos2.y;
				pos[5]=pos2.z;

				col[0]=color.x;
				col[1]=color.y;
				col[2]=color.z;
				glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(pos),pos);
				positionID=	shaderProgram->GetAttribLocation("position");
				gWorldID=	shaderProgram->GetUniformLocation("gWVCP");
				PixelColorID= shaderProgram->GetUniformLocation("PixelColor");
				PointSizeID= shaderProgram->GetUniformLocation("size");
			}
Line::~Line()
			{
			    if(shaderProgram!=0)
                    delete shaderProgram;
			}
void Line::Render(Camera* pGameCamera, int width, int height)
			{
				Assistant TM;
				TM.WorldPos(0,0,0);
				TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
				TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);


                shaderProgram->Use();
				//glUseProgram(shaderProgramID);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glVertexAttribPointer(positionID,3,GL_FLOAT,GL_FALSE,0,0);


				glUniformMatrix4fv(gWorldID, 1, GL_TRUE, (const GLfloat*)TM.GetTSRVC());
				glUniform1f(PointSizeID,1.5);
				glUniform4f(PixelColorID, col[0],col[1],col[2],1.0);
				glEnableVertexAttribArray(positionID);
				glDrawArrays(GL_LINES,0,2);
				glDisableVertexAttribArray(positionID);
			}
Shader* Line::GetShader()
{
    return shaderProgram;
}
