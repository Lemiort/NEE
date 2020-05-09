#version 460

layout(location = 0) in vec3 vertex_position;

uniform mat4 gWVP;

out vec3 TexCoord0;
void main() {
    vec4 WVP_Pos = gWVP * vec4(vertex_position, 1.0);
    gl_Position = WVP_Pos.xyww;
    TexCoord0 = vertex_position;
}