#version 330                                                                  

uniform mat4 gWorld;
uniform mat4 gVC;
uniform mat4 mRotate;
in mat4 instanceWorld;
in mat4 instanceRotation;
uniform bool instanced;

                                                                             
in vec3 s_vPosition;
in vec4 s_vNormal;
in vec2 s_vUV;
in vec3 s_vTangent;
uniform vec3 s_vCamPos; 

uniform vec3 dLightDir; 
uniform vec3 pLightPos;
uniform vec3 sLightPos;
//uniform vec3 sLightDir;
uniform float pLightInt;



 out vec3 fN; //нормаль
 out vec3 fL; //вектор направленного света
 out vec3 fE; //вектор взгляда
 out vec2 UV;
 out vec3 pL;//вектор точечного света
 out vec3 sR;//вектор от источника до точки
 out vec3 sLD;//вектор направления источника

 out vec3 Tangent;
 out vec3 Bitangent;
 out vec3 fT;
 out vec3 fB;
 out float power;   
                                                                                                      
void main()                                                                   
{                                                                             
	mat4 world = instanced ? instanceWorld : gWorld;
	mat4 rotation = instanced ? instanceRotation : mRotate;
	vec3 worldPosition = (world * vec4(s_vPosition, 1.0)).xyz;
	gl_Position = gVC * vec4(worldPosition, 1.0);
	fN=(rotation*s_vNormal).xyz;
	fL=-dLightDir;
	fE=worldPosition-s_vCamPos;
	UV=s_vUV;

	//длина вектора от света до точки
	float temp=max(length(pLightPos-worldPosition), 0.001);
	power=pLightInt/(temp*temp);
	pL=pLightPos-worldPosition;
	sR=worldPosition-sLightPos;

	Tangent=normalize((world * vec4(s_vTangent, 0.0)).xyz);
}
