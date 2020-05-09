#version 460
uniform mat4 gWVCP;
uniform float size;

in vec3 position;

void main() {
    gl_Position = gWVCP * vec4(position, 1.0);
    gl_PointSize = size;
}