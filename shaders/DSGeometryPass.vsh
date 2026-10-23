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

 out vec3 Tangent;
 out vec2 UV;
 out vec3 fN; //нормаль
 out vec4 Position;
 
void main()                                                                   
{                                                                             
	mat4 world = instanced ? instanceWorld : gWorld;
	mat4 rotation = instanced ? instanceRotation : mRotate;
	gl_Position = gVC*world *vec4(s_vPosition, 1.0);
	//gl_Position = gWorld *vec4(s_vPosition, 1.0);	
	fN=(rotation*s_vNormal).xyz;
	//fL=-dLightDir;
	//fE=(gVC*gWorld*vec4(s_vCamPos,1)).xyz;
	UV=s_vUV;
	Position  = (world*vec4(s_vPosition,1.0));

	/*float temp=length(-(gWorld*vec4(1,1,1,1.0)).xyz+pLightPos);
	power=pLightInt*pow(temp,-2);
	pL=(-(gWorld*vec4(1,1,1,1.0)).xyz+pLightPos);
	sR=((gWorld*vec4(1,1,1,1.0)).xyz-sLightPos);*/

	Tangent=normalize((world * vec4(s_vTangent, 0.0)).xyz);
}
