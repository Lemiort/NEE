#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include <glad/gl.h>
// gl goes first
#include <GLFW/glfw3.h>

#include <memory>
#include <string>

#include "Billboard.hpp"
#include "GBuffer.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "PerlinNoise.hpp"
#include "ShadowMapFBO.hpp"
#include "Skybox.hpp"
#include "Text2D.hpp"
#include "TextRenderer.hpp"

constexpr int WINDOW_WIDTH = 1366;
constexpr int WINDOW_HEIGHT = 768;

enum class RenderType : int {
    DEFERRED_SHADING = 0,
    POSITION_BUFFER = 1,
    DIFFUSE_BUFFER = 2,
    NORMAL_BUFFER = 3,
    TEXCOORD_BUFFER = 4,
    FORWARD_RENDERING = 5
};

class Application {
public:
    Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int Run(GLFWwindow* window);
    static void ErrorCallback(int error, const char* description);

private:
    struct Mouse {
        bool rightButtonPressed = false;
        bool leftButtonPressed = false;
        double posX = 0.0;
        double posY = 0.0;
        double dx = 0.0;
        double dy = 0.0;

        void Update(double x, double y) {
            dx = x - posX;
            dy = y - posY;
            posX = x;
            posY = y;
        }
    };

    static void KeyCallback(GLFWwindow* window, int key, int scancode,
                            int action, int mods);
    static void MousePosCallBack(GLFWwindow* window, double x, double y);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action,
                                    int mods);
    static void FrameBufferSizeCallback(GLFWwindow* window, int w, int h);

    void OnKey(GLFWwindow* window, int key, int action);
    void OnMousePos(double x, double y);
    void OnMouseButton(int button, int action);
    void OnFrameBufferSize(int w, int h);
    void CalcFPS();
    void PreInitScene(GLFWwindow* window);
    void InitRender(GLFWwindow* window, std::string message);
    int InitScene(GLFWwindow* window);
    void RenderScene();
    void DSLightingPass();
    void RenderPass();
    void ShadowPass();
    void DSGeometryPass();
    static void DSBeginLightPasses();
    void DSStencilPass(Light& light);
    void DSSpotLightPass(SpotLight& spotLight);
    void DSPointLightPass(PointLight& pointLight);
    void DSDirectionalLightPass(DirectionalLight& directionalLight);
    void DSFinalPass();
    static void DSEndLigtPasses();
    void InterfacePass();

    int width = WINDOW_WIDTH;
    int height = WINDOW_HEIGHT;
    Mouse mouse;
    Camera pGameCamera{WINDOW_WIDTH, WINDOW_HEIGHT, 30.0F, 0.1F, 1000.0F};
    float Scale = 0.0F;
    float fps = 0.0F;
    int frameCount = 0;
    double lastTime = 0.0;
    RenderType renderType = RenderType::DEFERRED_SHADING;
    bool initialized = false;

    GLint gCamViewID = 0, rotateID = 0, camPosID = 0;
    GLint dirLightDirID = 0, dirLightColID = 0;
    GLint pointLightColID = 0, pointLightIntID = 0, pointLightPosID = 0;
    GLint spotLightColID = 0, spotLightDirID = 0, spotLightCutoffID = 0,
          spotLightPosID = 0;

    std::shared_ptr<Shader> meshShader, skyboxShader, textShader, shadowShader,
        shadowMeshShader;
    std::shared_ptr<Shader> DSGeometryPassShader, DSPointLightShader;
    std::shared_ptr<Shader> DSDirectionalLightShader, DSSpotLightShader,
        DSStencilPassShader;
    std::shared_ptr<Material> mainMaterial, shadowMaterial, shadowMeshMaterial;
    std::shared_ptr<Material> DSGeometryPassMaterial, DSPointLightMaterial;
    std::shared_ptr<Material> DSDirectionalLightMaterial, DSSpotLightMaterial,
        DSStencilPassMaterial;
    std::shared_ptr<Texture2D> whiteTexture;
    std::unique_ptr<GBuffer> gBuffer1;
    std::unique_ptr<DirectionalLight> directionalLight1;
    std::unique_ptr<PointLight> pointLight1, pointLight2;
    std::unique_ptr<SpotLight> spotLight1;
    std::unique_ptr<Line> xline, yline, zline, dirLightLine;
    Mesh TestMesh, Plane, Sphere;
    std::unique_ptr<SkyBox> skybox1;
    std::shared_ptr<Font2d> interfaceFont;
    std::unique_ptr<TextRenderer> textRenderer;
    std::unique_ptr<FontLine2d> fLine1;
    std::unique_ptr<PerlinNoise> noise1;
    std::unique_ptr<Billboard> bb1;
    std::unique_ptr<ShadowMapFBO> smfbo1;
};

#endif  // MAIN_H_INCLUDED
