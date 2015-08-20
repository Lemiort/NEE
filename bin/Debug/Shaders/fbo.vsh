#version 330                                                                  

uniform mat4 gWorld;
uniform mat4 gVC;

                                                                             
in vec3 s_vPosition;
in vec4 s_vNormal;
in vec2 s_vUV;
in vec3 s_vTangent;


                                                                                                      
void main()                                                                   
{                                                                             
	gl_Position = gVC*gWorld *vec4(s_vPosition, 1.0);	
}