#include "Light.h"

Line::Line(Vector3f pos1, Vector3f pos2, Vector3f color)
			{
				char* vertexShaderSorceCode=ReadFile("Shaders/lightVS.vsh");
				char* fragmentShaderSourceCode=ReadFile("Shaders/lightFS.fsh");
				GLuint vertexShaderID=MakeVertexShader(vertexShaderSorceCode);
				GLuint fragmentShaderID=MakeFragmentShader(fragmentShaderSourceCode);
				shaderProgramID=MakeShaderProgram(vertexShaderID, fragmentShaderID);

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
				positionID=	glGetAttribLocation(shaderProgramID,"position");
				gWorldID=	glGetUniformLocation(shaderProgramID, "gWVCP");
				PixelColorID= glGetUniformLocation(shaderProgramID, "PixelColor");
				PointSizeID= glGetUniformLocation(shaderProgramID, "size");
			}
Line::Line(Vector3f pos1, Vector3f pos2, Vector3f color,GLuint shader)
			{
				shaderProgramID=shader;
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
				positionID=	glGetAttribLocation(shaderProgramID,"position");
				gWorldID=	glGetUniformLocation(shaderProgramID, "gWVCP");
				PixelColorID= glGetUniformLocation(shaderProgramID, "PixelColor");
				PointSizeID= glGetUniformLocation(shaderProgramID, "size");
			}
Line::~Line()
			{
			}
void Line::Render(Camera* pGameCamera, int width, int height)
			{
				Assistant TM;
				TM.WorldPos(0,0,0);
				TM.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
				TM.SetPerspectiveProj(30.0f, width, height, 1.0f, 1000.0f);

				

				glUseProgram(shaderProgramID);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glVertexAttribPointer(positionID,3,GL_FLOAT,GL_FALSE,0,0);
				

				glUniformMatrix4fv(gWorldID, 1, GL_TRUE, (const GLfloat*)TM.GetTSRVC());
				glUniform1f(PointSizeID,1.5);
				glUniform4f(PixelColorID, col[0],col[1],col[2],1.0);
				glEnableVertexAttribArray(positionID);
				glDrawArrays(GL_LINES,0,2);
				glDisableVertexAttribArray(positionID);
			}