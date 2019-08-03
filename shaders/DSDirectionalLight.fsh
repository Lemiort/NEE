#version 330                   



in vec3 fN;//нормалька
in vec2 UV;//координаты текстуры
in vec3 fE;//направление взгляда

in vec3 Tangent;//тангент для нормали

in vec4 screenSpacePos;//позиция точки в экранных координатах
//in vec4 pointLightPos;//позиция света в мировых координата



uniform mat4 dirLightColor;//цвет
uniform vec3 dirLightDirection;//направление света


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


vec4 CalcBaseLight(mat4 color)
{
	vec2 screenPosUV = CalcTexCoord();
	vec4 screenPos= texture(gSampler4, screenPosUV);//позиция точки на экране

	vec3 N = texture(gSampler6, screenPosUV).xyz;//нормаль
	vec3 L=normalize(dirLightDirection);
	vec3 E=normalize(-fE);
	vec3 H=normalize(E+L);
	
	float diffuse_intensity=max(dot(N,L),0.0);
	float spec_intensity=pow(max(dot(N,H),0.0),300);
	vec4 result=color*(diffuse_intensity*texture2D(gSampler5, screenPosUV)+spec_intensity*texture2D(gSampler8, screenPosUV));
	return result;
}

void main()
{
	//vec2 screenPosUV = CalcTexCoord();
	//vec4 screenPos= texture(gSampler8, screenPosUV);//позиция точки на экране
	fColor0 = CalcBaseLight(dirLightColor);
	//fColor0 = screenPos;
	//fColor0 = vec4(normalize(fN), 1.0);
	fColor0.a = 1.0;

}