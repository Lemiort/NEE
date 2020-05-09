#version 460

in vec3 fN;  //нормаль
in vec2 UV;
in vec4 Position;  //позиция в мировых координатах

in vec3 Tangent;
// текстура
uniform sampler2D colTexSampler;
uniform sampler2D normTexSampler;
uniform sampler2D specTexSampler;

/*layout(location = 0) out	vec4 oWorldPos;
layout(location = 1) out	vec4 oDiffuse;
layout(location = 2) out	vec4 oNormal;
layout(location = 3) out	vec4 oTexCoord;*/
layout(location = 0) out vec4 fColor0;  // worldPos
layout(location = 1) out vec4 fColor1;  // diffuse
layout(location = 2) out vec4 fColor2;  // normal
layout(location = 3) out vec4 fColor3;  // texCoord
layout(location = 4) out vec4 fColor4;  // specular

vec3 CalcBumpedNormal() {
    vec3 Normal = normalize(fN);
    vec3 Tangent1 = normalize(Tangent);
    Tangent1 = normalize(Tangent1 - dot(Tangent1, Normal) * Normal);
    vec3 Bitangent = cross(Tangent1, Normal);
    vec3 BumpMapNormal = texture(normTexSampler, UV).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent1, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}

void main() {
    vec3 Normal = CalcBumpedNormal();

    /*oNormal.xyz = Normal;
    oWorldPos.xyz = gl_FragCoord.xyz;
    oDiffuse.xyz = texture(colTexSampler, UV).xyz;
    oTexCoord.xyz = vec3(UV,0.0);

    oWorldPos.a	= 1.0;
    oDiffuse.a = 1.0;
    oNormal.a = 1.0;
    oTexCoord.a = 1.0;*/

    // fColor0 = gl_FragCoord;
    // fColor0 = vec4(normalize(Position), 1 / length(Position));
    /*if(length(Position) > 1.0 )
            fColor0 = vec4( normalize(Position) ,1 / length(Position));
    else*/
    fColor0 = Position;

    fColor1 = vec4(texture(colTexSampler, UV).xyz, 1.0);
    fColor2 = vec4(Normal, 1.0);
    fColor3 = vec4(UV, 0.0, 1.0);
    fColor4 = vec4(texture(specTexSampler, UV).xyz, 1.0);
}