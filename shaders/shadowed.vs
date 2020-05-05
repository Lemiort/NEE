#version 330

uniform mat4 gWorld;
uniform mat4 gVC;
uniform mat4 mRotate;

uniform mat4 gLightVC;

in vec3 s_vPosition;
in vec4 s_vNormal;
in vec2 s_vUV;
in vec3 s_vTangent;
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

out vec4 LightSpacePos;

void main() {
    gl_Position = gVC * gWorld * vec4(s_vPosition, 1.0);
    LightSpacePos = gLightVC * gWorld * vec4(s_vPosition, 1.0);

    fN = (mRotate * s_vNormal).xyz;
    fL = -dLightDir;
    fE = (gVC * gWorld * vec4(s_vCamPos, 1)).xyz;
    UV = s_vUV;

    float temp = length(-(gWorld * vec4(1, 1, 1, 1.0)).xyz + pLightPos);
    power = pLightInt * pow(temp, -2);
    pL = (-(gWorld * vec4(1, 1, 1, 1.0)).xyz + pLightPos);
    sR = ((gWorld * vec4(1, 1, 1, 1.0)).xyz - sLightPos);

    Tangent = normalize((gWorld * vec4(s_vTangent, 0.0)).xyz);
}