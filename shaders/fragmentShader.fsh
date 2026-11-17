#version 460 core


in vec3 fN;//направление нормали
in vec3 fL;//направление направленного света
in vec3 fE;//направление взгляда
in vec2 UV;

 in vec3 pL;//направление точечного света
 in float power;//сила точечного света
 in vec3 sR;//направление прожектора
 //in vec3 sLD;

 in vec3 Tangent;
 in vec3 worldPos;
// текстура
uniform sampler2D colTexSampler;
uniform sampler2D normTexSampler;
uniform sampler2D specTexSampler;
uniform mat4 dLightCol;
uniform mat4 pLightCol;
uniform mat4 sLightCol;
uniform vec3 sLightDir;

uniform float sLightCutoff;
uniform float opacity;
uniform sampler2D emissiveTexSampler;
uniform vec3 emissiveColor;
uniform vec2 worldTextureScale;

out vec4 fColor;

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

// Set once in main() before any lighting.
vec2 surfaceUV;

// normalize() of a zero vector is NaN; a zero direction lights nothing.
vec3 SafeNormalize(vec3 v)
{
	float len = length(v);
	return len > 1e-6 ? v / len : vec3(0.0);
}

vec4 CalcBaseLight(vec3 dir,vec3 Normal, mat4 color)
{
	vec3 N=(Normal);
	vec3 L=SafeNormalize(dir);
	vec3 E=SafeNormalize(-fE);
	vec3 H=SafeNormalize(E+L);
	float diffuse_intensity=max(dot(N,L),0.0);
	float spec_intensity=pow(max(dot(N,H),0.0),300);
	vec4 result=color*(diffuse_intensity*texture(colTexSampler, surfaceUV)+spec_intensity*texture(specTexSampler, surfaceUV));
	
	return result;
}

vec4 CalcSpotLight(vec3 dir,vec3 Normal, mat4 color)
{
	if(length(dir) <= 1e-6 || length(sR) <= 1e-6)
		return vec4(0.0);
	vec3 N=(Normal);
	vec3 R=normalize(sR);
	vec3 L=-R;
	vec3 E=SafeNormalize(-fE);
	vec3 H=SafeNormalize(E+L);
	float diffuse_intensity=max(dot(N,L),0.0);
	float spec_intensity=pow(max(dot(N,H),0.0),300);
	float SpotFactor=dot(R,normalize(dir));
	if(SpotFactor <= sLightCutoff)
		return vec4(0.0);
	vec4 result=color*(diffuse_intensity*texture(colTexSampler, surfaceUV)+spec_intensity*texture(specTexSampler, surfaceUV));
	return result*(1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - sLightCutoff));
}

vec3 CalcBumpedNormal()
{
	vec3 Normal = normalize(fN);
	vec3 Tangent1 = normalize(Tangent);
	Tangent1 = normalize(Tangent1 - dot(Tangent1, Normal) * Normal);
	vec3 Bitangent = cross(Tangent1, Normal);
	vec3 BumpMapNormal = texture(normTexSampler, surfaceUV).xyz;
	BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
	vec3 NewNormal;
	mat3 TBN = mat3(Tangent1, Bitangent, Normal);
	NewNormal = TBN * BumpMapNormal;
	NewNormal = normalize(NewNormal);
	return NewNormal;
}


void main()
{
	surfaceUV = SurfaceUV(worldPos, fN, UV);
	vec3 Normal=CalcBumpedNormal();
	//vec3 Normal=normalize(fN);
	//направленный свет
	vec4 dirLightColor=CalcBaseLight(fL,Normal,dLightCol);
	//точечный свет
	vec4 pointLightColor=CalcBaseLight(pL,Normal,pLightCol)*power;
	//свет прожектора
	vec4 spotLightColor=CalcSpotLight(sLightDir,Normal,sLightCol);
	fColor=pointLightColor+dirLightColor+spotLightColor;
	fColor.rgb+=texture(emissiveTexSampler, surfaceUV).rgb*emissiveColor;
	fColor.a=opacity;
	//fColor=vec4(Tangent,1.0);
	/*if(fColor.x==0 && fColor.y==0 && fColor.z==0) 
	{
		fColor=vec4(1.0,0.01,0.01,0.5);
	}*/
}
