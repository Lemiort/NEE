#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
#include <glad/gl.h>
// gl goes first
#include <GLFW/glfw3.h>

#include <memory>
#include <string>

#include "Billboard.hpp"
#include "DebugRenderer.hpp"
#include "DeferredPipeline.hpp"
#include "ForwardPipeline.hpp"
#include "GBuffer.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "PerlinNoise.hpp"
#include "RenderMode.hpp"
#include "RenderQueue.hpp"
#include "Scene.hpp"
#include "Skybox.hpp"
#include "Text2D.hpp"
#include "TextRenderer.hpp"

constexpr int WINDOW_WIDTH = 1366;
constexpr int WINDOW_HEIGHT = 768;

class Application {
public:
    Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int Run(GLFWwindow& window);
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

    void OnKey(GLFWwindow& window, int key, int action);
    void OnMousePos(double x, double y);
    void OnMouseButton(int button, int action);
    void OnFrameBufferSize(int w, int h);
    void CalcFPS();
    void PreInitScene(GLFWwindow& window);
    void InitRender(GLFWwindow& window, std::string message);
    int InitScene(GLFWwindow& window);
    void RenderScene();
    void InterfacePass();

    int width = WINDOW_WIDTH;
    int height = WINDOW_HEIGHT;
    Mouse mouse;
    Camera pGameCamera{WINDOW_WIDTH, WINDOW_HEIGHT, 30.0F, 0.1F, 1000.0F};
    float animationTime_ = 0.0F;
    float fps = 0.0F;
    int frameCount = 0;
    double lastTime = 0.0;
    RenderMode renderMode = RenderMode::DeferredShading;
    bool initialized = false;

    std::shared_ptr<Shader> meshShader, skyboxShader, textShader, debugShader;
    std::shared_ptr<Shader> DSGeometryPassShader, DSPointLightShader;
    std::shared_ptr<Shader> DSDirectionalLightShader, DSSpotLightShader,
        DSStencilPassShader;
    std::shared_ptr<Material> mainMaterial;
    std::shared_ptr<Material> DSPointLightMaterial;
    std::shared_ptr<Material> DSDirectionalLightMaterial, DSSpotLightMaterial,
        DSStencilPassMaterial;
    std::shared_ptr<GBuffer> gBuffer1;
    std::unique_ptr<DeferredPipeline> deferredPipeline;
    std::unique_ptr<DebugRenderer> debugRenderer;
    std::unique_ptr<ForwardPipeline> forwardPipeline;
    std::shared_ptr<Font2d> interfaceFont;
    std::unique_ptr<TextRenderer> textRenderer;
    std::unique_ptr<FontLine2d> fLine1;
    Scene scene;
    RenderQueue renderQueue;
};

#endif  // MAIN_H_INCLUDED
