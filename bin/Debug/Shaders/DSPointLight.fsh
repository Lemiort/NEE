#version 330                   



in vec3 fN;//нормалька
in vec2 UV;//координаты текстуры
in vec3 fE;//направление взгляда

in vec3 Tangent;//тангент для нормали

in vec4 screenSpacePos;//позиция точки в экранных координатах
//in vec4 pointLightPos;//позиция света в мировых координата


uniform  float pointLightIntensity;//интенсивность
uniform mat4 pointLightColor;//цвет
uniform vec3 s_vPointLightPos;//позиция света


// текстура
uniform sampler2D colTexSampler;
uniform sampler2D normTexSampler;
uniform sampler2D specTexSampler;

uniform sampler2D gSampler4; //world pos
uniform sampler2D gSampler5; //diffuse
uniform sampler2D gSampler6; //normal
uniform sampler2D gSampler7; //uv
uniform sampler2D gSampler8; //specular




layout(location = 0) out vec4 fColor0;



vec2 CalcTexCoord()//считаем позицию света на текустуре
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


vec4 CalcPointLight(mat4 lightColor)
{

	vec2 screenPosUV = CalcTexCoord();
	vec4 screenPos= texture(gSampler4, screenPosUV);//позиция точки на экране
	if(screenPos  == vec4 (0.0,0.0,0.0,0.0))
		return vec4(0.0);
	else
	{
		
		float distance = distance(screenPos.xyz,s_vPointLightPos.xyz);
		
		vec3 N = texture(gSampler6, screenPosUV).xyz;//нормаль
		vec3 L = normalize(-(screenPos.xyz - s_vPointLightPos.xyz));//направление света
		vec3 E = normalize(-fE);//направление взгляда
		vec3 H = normalize(E+L);
		
		float diffuse_intensity=max(dot(N,L),0.0);
		float spec_intensity=pow(max(dot(N,H),0.0),300);
		
		float attenuation = distance * distance;
		//attenuation = max(1.0, attenuation);
		
		//vec4 result =texture2D(gSampler8, screenPosUV);
		vec4 result;
		result=pointLightIntensity*lightColor*(diffuse_intensity*texture2D(gSampler5, screenPosUV)+spec_intensity*texture2D(gSampler8, screenPosUV));

		return result / attenuation;
	}
}

void main()
{
	
	//fColor0= CalcBaseLight(normalize(pointLightPos-gl_FragCoord).xyz, fN, pointLightColor);
	fColor0 = CalcPointLight(pointLightColor);
	//fColor0.xyz = pointLightPos;
	fColor0.a = 1.0;

}