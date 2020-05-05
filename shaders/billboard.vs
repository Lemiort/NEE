#version 330
layout(location = 0) in vec3 s_vPosition;

void main() {
    gl_Position = vec4(s_vPosition, 1.0);
    gl_PointSize = 10.0;
}