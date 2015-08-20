#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include <GL\glew.h>
#include <iostream>
#include <fstream>
#include <conio.h>
#include <string.h>
#include "Logger.h"
#include "Light.h"
#include "Mesh.h"
#include "tga_loader.h"
#include "skybox.h"
#include "Text_2D.h"
#include "PerlinNoise.h"
#include "Billboard.h"
#include <Material.h>
#include "ShadowMapFBO.h"
#include <sstream>
#include <version.h>
#include <GBuffer.h>
#include <util.h>

#define BUFFER_OFFSET(i) ((char*)NULL +(i))
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

/*
typedef __int8           int8_t;
typedef unsigned __int8  uint8_t;
typedef __int16          int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32          int32_t;
typedef unsigned __int32 uint32_t;

*/
Shader* meshShader;
Shader* skyboxShader;
Shader* textShader;
Shader* lightShader;
Shader* shadowShader;
Shader* shadowMeshShader;
Shader* DSGeometryPassShader;
Shader* DSPointLightShader;
Shader* DSDirectionalLightShader;
Shader* DSSpotLightShader;

GBuffer* gBuffer1;

#endif // MAIN_H_INCLUDED
