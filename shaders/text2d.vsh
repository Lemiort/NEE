#version 330  
in vec2 Position;//координата точки
uniform vec2 s_Position;//положение на экране
uniform vec2 s_UV;
uniform float size;
in vec2 UV;
out vec2 UV1;
void main()
{
	vec2 temp=(Position*size)+s_Position;

	gl_Position=vec4(temp,-1,1.0);
	//gl_Position=vec4(0,0,0,1);
	UV1=UV+s_UV;
}