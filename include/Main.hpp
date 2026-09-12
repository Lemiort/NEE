#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

#include "Billboard.hpp"
#include "GBuffer.hpp"
#include "Light.hpp"
#include "Logger.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "PerlinNoise.hpp"
#include "ShadowMapFBO.hpp"
#include "Skybox.hpp"
#include "Text2D.hpp"
#include "TgaLoader.hpp"
#include "Util.hpp"
#include "version.hpp"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))
#define WINDOW_WIDTH 1366
#define WINDOW_HEIGHT 768

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

/*
typedef __int8           int8_t;
typedef unsigned __int8  uint8_t;
typedef __int16          int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32          int32_t;
typedef unsigned __int32 uint32_t;

*/
shared_ptr<Shader> meshShader;
shared_ptr<Shader> skyboxShader;
shared_ptr<Shader> textShader;
shared_ptr<Shader> lightShader;
shared_ptr<Shader> shadowShader;
shared_ptr<Shader> shadowMeshShader;
shared_ptr<Shader> DSGeometryPassShader;
shared_ptr<Shader> DSPointLightShader;
shared_ptr<Shader> DSDirectionalLightShader;
shared_ptr<Shader> DSSpotLightShader;
shared_ptr<Shader> DSStencilPassShader;

shared_ptr<Material> shadowMeshMaterial;
shared_ptr<Material> DSGeometryPassMaterial;
shared_ptr<Material> DSPointLightMaterial;
shared_ptr<Material> DSDirectionalLightMaterial;
shared_ptr<Material> DSSpotLightMaterial;
shared_ptr<Material> DSStencilPassMaterial;

std::unique_ptr<GBuffer> gBuffer1;
GLFWwindow* hiddenWindow;

struct Mouse {
    bool rightButtonPressed;
    bool leftButtonPressed;
    double posX;
    double posY;
    double dx, dy;
    Mouse() {
        rightButtonPressed = false;
        leftButtonPressed = false;
        posX = 0.0f;
        posY = 0.0f;
        dx = 0.0f;
        dy = 0.0f;
    }
    void Update(double x, double y) {
        dx = x - posX;
        dy = y - posY;
        posX = x;
        posY = y;
    }
} mouse;

void PreInitScene(GLFWwindow* window);
void InitRender(GLFWwindow* window, string message);
int InitScene(GLFWwindow* window);

void RenderScene(GLFWwindow* window);

void DSLightingPass();

void RenderPass();
void ShadowPass();

void DSGeometryPass();
void DSBeginLightPasses();
void DSStencilPass(Light& light);
void DSSpotLightPass(SpotLight& spotLight);
void DSPointLightPass(PointLight& pointLight);
void DSDirectionalLightPass(DirectionalLight& directionalLight);
void DSFinalPass();
void DSEndLigtPasses();
void InterfacePass();

void FrameBufferSizeCallback(GLFWwindow* window, int w, int h);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MousePosCallBack(GLFWwindow* window, double x, double y);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods);
void ErrorCallback(int error, const char* description);
void CalcFPS();

int width;
int height;

GLuint gWorldID, gCamViewID;
GLuint rotateID;
GLuint dirLightDirID, dirLightColID;
GLuint pointLightColID, pointLightIntID, pointLightPosID;
GLuint spotLightColID, spotLightDirID, spotLightCutoffID, spotLightPosID;
GLuint camtransID, camPosID;

std::unique_ptr<DirectionalLight> directionalLight1;
std::unique_ptr<PointLight> pointLight1, pointLight2;
std::unique_ptr<SpotLight> spotLight1;
static Camera pGameCamera(WINDOW_WIDTH, WINDOW_HEIGHT, 30.0f, 0.1f, 1000.0f);
float Scale;
// GLfloat light[]= {0.0,1.0,-1.0,1.0};
int spfaces;
int spverts;
std::unique_ptr<Line> xline;
std::unique_ptr<Line> yline;
std::unique_ptr<Line> zline;
std::unique_ptr<Line> dirLightLine;
Mesh TestMesh, Plane, Cube;
std::unique_ptr<SkyBox> skybox1;
std::unique_ptr<TextLine2d> tline1, tline2;
std::unique_ptr<FontLine2d> fLine1;
std::unique_ptr<PerlinNoise> noise1;
std::unique_ptr<Billboard> bb1;
std::unique_ptr<ShadowMapFBO> smfbo1;

// signal that shader loading and other tasks are complete
bool initialized;

// message passed between two threads
string sharedMessage;
shared_ptr<Material> mainMaterial, secondMaterial, shadowMaterial;
shared_ptr<Texture2D> colorMap1, colorMap2, whiteTexture;
float fps;
int frameCount;
double lastTime;
int renderType;

#endif  // MAIN_H_INCLUDED
