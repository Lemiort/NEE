#version 460

uniform mat4 model;
uniform mat4 view_projection;
uniform mat4 model_rotation;

in vec3 vertex_position;
in vec4 vertex_normal;
in vec2 vertex_uv;
in vec3 vertex_tangent;
uniform vec3 s_vCamPos;

uniform vec3 dLightDir;
uniform vec3 pLightPos;
uniform vec3 sLightPos;
// uniform vec3 sLightDir;
uniform float pLightInt;

out vec3 fN;  //нормаль
out vec3 fL;  //вектор направленного света
out vec3 fE;  //вектор взгляда
out vec2 UV;
out vec3 pL;   //вектор точечного света
out vec3 sR;   //вектор от источника до точки
out vec3 sLD;  //вектор направления источника

out vec3 Tangent;
out vec3 Bitangent;
out vec3 fT;
out vec3 fB;
out float power;

void main() {
    gl_Position = view_projection * model * vec4(vertex_position, 1.0);
    fN = (model_rotation * vertex_normal).xyz;
    fL = -dLightDir;
    fE = (view_projection * model * vec4(s_vCamPos, 1)).xyz;
    UV = vertex_uv;

    //длина вектора от света до точки
    float temp = length(-(model * vec4(1, 1, 1, 1.0)).xyz + pLightPos);
    power = pLightInt * pow(temp, -2);
    pL = (-(model * vec4(1, 1, 1, 1.0)).xyz + pLightPos);
    sR = ((model * vec4(1, 1, 1, 1.0)).xyz - sLightPos);

    Tangent = normalize((model * vec4(vertex_tangent, 0.0)).xyz);
}