#version 460 core



in vec3 fN;//нормаль
in vec2 UV;
in vec4 Position; //позиция в мировых координатах

in vec3 Tangent;
// текстура
uniform sampler2D colTexSampler;
uniform sampler2D normTexSampler;
uniform sampler2D specTexSampler;
uniform sampler2D emissiveTexSampler;
uniform vec3 emissiveColor;
uniform vec2 worldTextureScale;


/*layout(location = 0) out	vec4 oWorldPos;
layout(location = 1) out	vec4 oDiffuse;
layout(location = 2) out	vec4 oNormal;
layout(location = 3) out	vec4 oTexCoord;*/
layout(location = 0) out vec4 fColor0;//worldPos
layout(location = 1) out vec4 fColor1;//diffuse
layout(location = 2) out vec4 fColor2;//normal
layout(location = 3) out vec4 fColor3;//texCoord
layout(location = 4) out vec4 fColor4;//specular
layout(location = 5) out vec4 fColor5;//emissive

// Texture coordinates from the world position when worldTextureScale is set:
// walls facing x use (z, y), walls facing z use (x, y), floors and roofs
// (x, z), so textures keep their size whatever the mesh's scale.
vec2 SurfaceUV(vec3 worldPosition, vec3 normal, vec2 meshUV)
{
	if (worldTextureScale == vec2(0.0))
		return meshUV;
	vec3 n = abs(normal);
	vec2 plane;
	if (n.x >= n.y && n.x >= n.z)
		plane = worldPosition.zy;
	else if (n.z >= n.y)
		plane = worldPosition.xy;
	else
		plane = worldPosition.xz;
	return plane * worldTextureScale;
}

vec3 CalcBumpedNormal(vec2 uv)
{
	vec3 Normal = normalize(fN);
	vec3 Tangent1 = normalize(Tangent);
	Tangent1 = normalize(Tangent1 - dot(Tangent1, Normal) * Normal);
	vec3 Bitangent = cross(Tangent1, Normal);
	vec3 BumpMapNormal = texture(normTexSampler, uv).xyz;
	BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
	vec3 NewNormal;
	mat3 TBN = mat3(Tangent1, Bitangent, Normal);
	NewNormal = TBN * BumpMapNormal;
	NewNormal = normalize(NewNormal);
	return NewNormal;
}


void main()
{
	vec2 uv = SurfaceUV(Position.xyz, fN, UV);
	vec3 Normal=CalcBumpedNormal(uv);
	
	/*oNormal.xyz = Normal;
	oWorldPos.xyz = gl_FragCoord.xyz;
	oDiffuse.xyz = texture(colTexSampler, UV).xyz;
	oTexCoord.xyz = vec3(UV,0.0);
	
	oWorldPos.a	= 1.0;
	oDiffuse.a = 1.0;
	oNormal.a = 1.0;
	oTexCoord.a = 1.0;*/
	
	//fColor0 = gl_FragCoord;
	//fColor0 = vec4(normalize(Position), 1 / length(Position));
	/*if(length(Position) > 1.0 )
		fColor0 = vec4( normalize(Position) ,1 / length(Position));
	else*/
	fColor0 = Position;
		
	fColor1 = vec4(texture(colTexSampler, uv).xyz, 1.0);
	fColor2 = vec4(Normal,1.0);
	fColor3 = vec4(uv,0.0,1.0);
	fColor4 = vec4(texture(specTexSampler, uv).xyz, 1.0);
	fColor5 = vec4(texture(emissiveTexSampler, uv).rgb * emissiveColor, 1.0);
}