#version 330   

uniform mat4 gWorld;
uniform mat4 gVC;
uniform mat4 mRotate;//вращение объекта, нужно ли оно?

                                                                             
in vec3 s_vPosition;
in vec4 s_vNormal;
in vec2 s_vUV;
in vec3 s_vTangent;

uniform vec3 s_vCamPos;

 out vec3 Tangent;
 out vec2 UV;
 out vec3 fN; //нормаль
 out vec3 fE; //направление взгляда
 
 out vec4 screenSpacePos;//позиция точки в экранных координатах
 
void main()                                                                   
{                  
	/*mat4 once;
	once[0][0] = 1.0;
	once[1][1] = 1.0;
	once[2][2] = 1.0;
	once[3][3] = 1.0;*/
	
	gl_Position = vec4(s_vPosition.x,s_vPosition.y, 0.0, 1.0);
	screenSpacePos = vec4(s_vPosition, 1.0);
	//gl_Position.z = 0.0;
	
	
	fN =(mRotate*s_vNormal).xyz;
	//fE=(gVC*gWorld*vec4(s_vCamPos,1)).xyz;
	fE = s_vCamPos;
	UV=s_vUV;
	
	Tangent=normalize((gWorld * vec4(s_vTangent, 0.0)).xyz);
}