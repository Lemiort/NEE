#version 460 core

uniform vec3 color;
uniform float opacity;

out vec4 fColor;

void main()
{
    fColor = vec4(color, opacity);
}
