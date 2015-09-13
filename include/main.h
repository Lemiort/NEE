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
#include <thread>

#define BUFFER_OFFSET(i) ((char*)NULL +(i))
#define WINDOW_WIDTH 1366
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
Shader* DSStencilPassShader;

Material* shadowMeshMaterial;
Material* DSGeometryPassMaterial;
Material* DSPointLightMaterial;
Material* DSDirectionalLightMaterial;
Material* DSSpotLightMaterial;
Material* DSStencilPassMaterial;

GBuffer* gBuffer1;
GLFWwindow* hiddenWindow;

    struct Mouse
    {
        bool rightButtonPressed;
        bool leftButtonPressed;
        double posX;
        double posY;
        double dx, dy;
        Mouse()
        {
            rightButtonPressed=false;
            leftButtonPressed=false;
            posX=0.0f; posY=0.0f;
            dx=0.0f; dy=0.0f;
        }
        void Update(double x, double y)
        {
            dx=x-posX;
            dy=y-posY;
            posX=x;
            posY=y;
        }
    }mouse;

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
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void ErrorCallback(int error, const char* description);
    void CalcFPS();

    int width;
    int height;

    GLuint gWorldID, gCamViewID;
    GLuint rotateID;
    GLuint dirLightDirID, dirLightColID;
    GLuint pointLightColID, pointLightIntID, pointLightPosID;
    GLuint spotLightColID, spotLightDirID, spotLightCutoffID,spotLightPosID;
    GLuint camtransID,camPosID;

    DirectionalLight* directionalLight1;
    PointLight* pointLight1,* pointLight2;
    SpotLight* spotLight1;
    Camera* pGameCamera = NULL;
    float Scale;
    //GLfloat light[]= {0.0,1.0,-1.0,1.0};
    int spfaces;
    int spverts;
    Line* xline;
    Line* yline;
    Line* zline;
    Line* dirLightLine;
    Mesh TestMesh,Plane,Cube;
    SkyBox* skybox1;
    TextLine2d* tline1,*tline2;
    FontLine2d* fLine1;
    PerlinNoise* noise1;
    Billboard* bb1;
    ShadowMapFBO* smfbo1;

    //сигнал о том, что прогрузка шейдеров и пр. завершилась
    bool initialized;

    //сообщение, передаваемое между двум потоками
    string sharedMessage;
    Material* mainMaterial,* secondMaterial,*shadowMaterial;
    Texture2D* colorMap1, *colorMap2, *whiteTexture;
    float fps;
    int frameCount;
    double lastTime;
    int renderType;


#endif // MAIN_H_INCLUDED
