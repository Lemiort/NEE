#version 330 

uniform sampler2D colTexSampler; 

in vec2 TexCoord; 
out vec4 FragColor; 

void main() 
{ 
FragColor = texture2D(colTexSampler, TexCoord); 

	if (FragColor.r == 0 && FragColor.g == 0 && FragColor.b == 0) 
	{ 
		discard; 
		//FragColor=vec4(1.0,0,0,1.0);
	} 
//FragColor=vec4(0.0,1.0,0,1.0);
}