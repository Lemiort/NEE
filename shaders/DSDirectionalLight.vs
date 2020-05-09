#version 460

uniform mat4 model;
uniform mat4 view_projection;
uniform mat4 model_rotation;  //вращение объекта, нужно ли оно?

in vec3 vertex_position;
in vec4 vertex_normal;
in vec2 vertex_uv;
in vec3 vertex_tangent;

uniform vec3 s_vCamPos;

out vec3 Tangent;
out vec2 UV;
out vec3 fN;  //нормаль
out vec3 fE;  //направление взгляда

out vec4 screenSpacePos;  //позиция точки в экранных координатах

void main() {
    /*mat4 once;
    once[0][0] = 1.0;
    once[1][1] = 1.0;
    once[2][2] = 1.0;
    once[3][3] = 1.0;*/

    gl_Position = vec4(vertex_position.x, vertex_position.y, 0.0, 1.0);
    screenSpacePos = vec4(vertex_position, 1.0);
    // gl_Position.z = 0.0;

    fN = (model_rotation * vertex_normal).xyz;
    // fE=(view_projection*model*vec4(s_vCamPos,1)).xyz;
    fE = s_vCamPos;
    UV = vertex_uv;

    Tangent = normalize((model * vec4(vertex_tangent, 0.0)).xyz);
}