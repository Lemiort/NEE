#version 330                   



in vec3 fN;//���������
in vec2 UV;//���������� ��������
in vec3 fE;//����������� �������

in vec3 Tangent;//������� ��� �������

in vec4 screenSpacePos;//������� ����� � �������� �����������
//in vec4 pointLightPos;//������� ����� � ������� ����������


uniform mat4 sLightCol;//����
uniform vec3 sLightPos;//������� �����
uniform vec3 sLightDir;//����������� �����
uniform float sLightCutoff; //cutoff


// ��������
uniform sampler2D colTexSampler;
uniform sampler2D normTexSampler;
uniform sampler2D specTexSampler;

uniform sampler2D gSampler4; //world pos
uniform sampler2D gSampler5; //diffuse
uniform sampler2D gSampler6; //normal
uniform sampler2D gSampler7; //uv
uniform sampler2D gSampler8; //specular




layout(location = 0) out vec4 fColor0;



vec2 CalcTexCoord()//������� ������� ����� �� ���������
{
	//vec3 ProjCoords = position.xyz / position.w;
	vec3 ProjCoords = screenSpacePos.xyz/screenSpacePos.w;
    vec2 UVCoords;
    //UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    //UVCoords.y = 0.5 * ProjCoords.y + 0.5;
	UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
	return UVCoords;
}

/*
vec4 CalcPointLight(mat4 lightColor)
{

	vec2 screenPosUV = CalcTexCoord();
	vec4 screenPos= texture(gSampler4, screenPosUV);//������� ����� �� ������
	if(screenPos  == vec4 (0.0,0.0,0.0,0.0))
		return vec4(0.0);
	else
	{
		
		float distance = distance(screenPos.xyz,s_vPointLightPos.xyz);
		
		vec3 N = texture(gSampler6, screenPosUV).xyz;//�������
		vec3 L = normalize(-(screenPos.xyz - s_vPointLightPos.xyz));//����������� �����
		vec3 E = normalize(-fE);//����������� �������
		vec3 H = normalize(E+L);
		
		float diffuse_intensity=max(dot(N,L),0.0);
		float spec_intensity=pow(max(dot(N,H),0.0),1);
		
		float attenuation = distance * distance;
		//attenuation = max(1.0, attenuation);
		
		//vec4 result =texture2D(gSampler8, screenPosUV);
		vec4 result;
		result=pointLightIntensity*lightColor*(diffuse_intensity*texture2D(gSampler5, screenPosUV)+spec_intensity*texture2D(gSampler8, screenPosUV));

		return result / attenuation;
	}
}*/

vec4 CalcSpotLight(mat4 color)
{

	vec2 screenPosUV = CalcTexCoord();
	vec4 screenPos= texture(gSampler4, screenPosUV);//������� ����� �� ������ � ������� �����������
	if(screenPos  == vec4 (0.0,0.0,0.0,0.0))
		return vec4(0.0);
	else
	{
		vec3 N = texture(gSampler6, screenPosUV).xyz;//�������
		vec3 L=normalize(-sLightDir);
		vec3 R=normalize(screenPos.xyz - sLightPos);//����������� ����� ���������� � ������ �����
		vec3 E=normalize(-fE);
		vec3 H=normalize(E+L);
		float diffuse_intensity=max(dot(N,L),0.0);
		float spec_intensity=pow(max(dot(N,H),0.0),300);
		float d=1.0/(1.0-sLightCutoff);
		float SpotFactor=dot(R,-L);
		
		if(SpotFactor > sLightCutoff)
		{
			vec4 result=color*(diffuse_intensity*texture2D(gSampler5, screenPosUV)+spec_intensity*texture2D(gSampler8, screenPosUV));
			return result*(1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - sLightCutoff));
		}
		else
		{
			return vec4(0.0,0.0,0.0,0.0);
		}
	}
}

void main()
{
	
	//fColor0= CalcBaseLight(normalize(pointLightPos-gl_FragCoord).xyz, fN, pointLightColor);
	fColor0 = CalcSpotLight(sLightCol);
	//fColor0.xyz += vec3(0.0,0.1,0.0);
	
	/*vec2 screenPosUV = CalcTexCoord();
	vec4 screenPos= texture(gSampler4, screenPosUV);//������� ����� �� ������
	fColor0 = vec4(normalize(screenPos.xyz - sLightPos),1.0);*/
	fColor0.a = 1.0;

}