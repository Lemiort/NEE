#version 330  
in vec2 UV1;
uniform sampler2D texSampler;

out vec4 FragColor;

void main()
{
 FragColor = texture(texSampler, UV1);
 //FragColor = vec4(1.0,0,0,1.0);
}