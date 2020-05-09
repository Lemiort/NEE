#version 460

uniform mat4 model;
uniform mat4 view_projection;

in vec3 vertex_position;
in vec4 vertex_normal;
in vec2 vertex_uv;
in vec3 vertex_tangent;

void main() {
    gl_Position = view_projection * model * vec4(vertex_position, 1.0);
}