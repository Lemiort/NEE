#version 460
uniform mat4 model;
uniform float size;

in vec3 position;

void main() {
    gl_Position = model * vec4(position, 1.0);
    gl_PointSize = size;
}