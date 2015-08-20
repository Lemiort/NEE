#version 330


in vec3 fN;
in vec3 fL;
in vec3 fE;
in vec2 UV;

 in vec3 pL;
 in float power;
 in vec3 sR;
 //in vec3 sLD;

 in vec3 Tangent;
// текстура
uniform sampler2D colTexSampler;
uniform sampler2D normTexSampler;
uniform sampler2D specTexSampler;
uniform mat4 dLightCol;
uniform mat4 pLightCol;
uniform mat4 sLightCol;
uniform vec3 sLightDir;

uniform float sLightCutoff;

out vec4 fColor;
vec4 CalcBaseLight(vec3 dir,vec3 Normal, mat4 color)
{
	vec3 N=(Normal);
	vec3 L=normalize(dir);
	vec3 E=normalize(-fE);
	vec3 H=normalize(E+L);
	float diffuse_intensity=max(dot(N,L),0.0);
	float spec_intensity=pow(max(dot(N,H),0.0),3000);
	vec4 result=color*(diffuse_intensity*texture2D(colTexSampler, UV)+spec_intensity*texture2D(specTexSampler, UV));
	return result;
}

vec4 CalcSpotLight(vec3 dir,vec3 Normal, mat4 color)
{
	vec3 N=(Normal);
	vec3 L=normalize(-dir);
	vec3 R=normalize(sR);
	vec3 E=normalize(-fE);
	vec3 H=normalize(E+L);
	float diffuse_intensity=max(dot(N,L),0.0);
	float spec_intensity=pow(max(dot(N,H),0.0),3000);
	float d=1.0/(1.0-sLightCutoff);
	float SpotFactor=dot(R,-L);
	vec4 result=color*(diffuse_intensity*texture2D(colTexSampler, UV)+spec_intensity*texture2D(specTexSampler, UV));
	return result*(1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - sLightCutoff));
}

vec3 CalcBumpedNormal()
{
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


void main()
{
	vec3 Normal=CalcBumpedNormal();
	//vec3 Normal=normalize(fN);
	//направленный свет
	vec4 dirLightColor=CalcBaseLight(fL,Normal,dLightCol);
	//точечный свет
	vec4 pointLightColor=CalcBaseLight(pL,Normal,pLightCol)*power;
	//свет прожектора
	vec4 spotLightColor=CalcSpotLight(sLightDir,Normal,sLightCol);
	fColor=pointLightColor+dirLightColor+spotLightColor;
	fColor.a=1.0;
	//fColor=vec4(Tangent,1.0);
	/*if(fColor.x==0 && fColor.y==0 && fColor.z==0) 
	{
		fColor=vec4(1.0,0.01,0.01,0.5);
	}*/
}