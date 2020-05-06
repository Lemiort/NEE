#version 330

uniform mat4 model;
uniform mat4 view_projection;
uniform mat4 model_rotation;

in vec3 vertex_position;
in vec4 vertex_normal;
in vec2 vertex_uv;
in vec3 vertex_tangent;

uniform vec3 s_vCamPos;
// uniform vec3 s_vPointLightPos;

out vec3 Tangent;
out vec2 UV;
out vec3 fN;  //нормаль
out vec3 fE;  //направление взгляда

// out vec4 pointLightPos; //позиция света в мировых координатах
out vec4 screenSpacePos;  //позиция точки в экранных координатах

void main() {
    // pointLightPos = vec4(s_vPointLightPos,1.0);
    screenSpacePos = view_projection * model * vec4(vertex_position, 1.0);
    gl_Position = screenSpacePos;

    fN = (model_rotation * vertex_normal).xyz;
    // fL=-dLightDir;
    // fE=(view_projection*model*vec4(s_vCamPos,1)).xyz;
    fE = s_vCamPos;
    UV = vertex_uv;

    /*float temp=length(-(model*vec4(1,1,1,1.0)).xyz+pLightPos);
    power=pLightInt*pow(temp,-2);
    pL=(-(model*vec4(1,1,1,1.0)).xyz+pLightPos);
    sR=((model*vec4(1,1,1,1.0)).xyz-sLightPos);*/

    Tangent = normalize((model * vec4(vertex_tangent, 0.0)).xyz);
}