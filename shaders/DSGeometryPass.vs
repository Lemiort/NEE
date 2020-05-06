#version 330

uniform mat4 model;
uniform mat4 view_projection;
uniform mat4 model_rotation;

in vec3 vertex_position;
in vec4 vertex_normal;
in vec2 vertex_uv;
in vec3 vertex_tangent;

out vec3 Tangent;
out vec2 UV;
out vec3 fN;  //нормаль
out vec4 Position;

void main() {
    gl_Position = view_projection * model * vec4(vertex_position, 1.0);
    // gl_Position = model *vec4(vertex_position, 1.0);
    fN = (model_rotation * vertex_normal).xyz;
    // fL=-dLightDir;
    // fE=(view_projection*model*vec4(s_vCamPos,1)).xyz;
    UV = vertex_uv;
    Position = (model * vec4(vertex_position, 1.0));

    /*float temp=length(-(model*vec4(1,1,1,1.0)).xyz+pLightPos);
    power=pLightInt*pow(temp,-2);
    pL=(-(model*vec4(1,1,1,1.0)).xyz+pLightPos);
    sR=((model*vec4(1,1,1,1.0)).xyz-sLightPos);*/

    Tangent = normalize((model * vec4(vertex_tangent, 0.0)).xyz);
}