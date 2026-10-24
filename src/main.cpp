#include "Main.hpp"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <initializer_list>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "Camera.hpp"
#include "Light.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Util.hpp"
#include "Version.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace {
constexpr int kGridExtentSteps = 50;
constexpr float kGridStepsPerUnit = 10.0F;
constexpr std::size_t kScreenshotFilenameCapacity = 80;
constexpr std::time_t kInvalidTime = static_cast<std::time_t>(-1);

struct ShaderStageSource {
    std::filesystem::path path;
    ShaderType type;
};

std::shared_ptr<Shader> CreateShader(
    std::initializer_list<ShaderStageSource> stages) {
    auto shader = std::make_shared<Shader>();
    for (const auto& [path, type] : stages) {
        shader->AddShader(ReadFile(path), type);
    }
    shader->Link();
    return shader;
}

void ConfigureLogging(const std::filesystem::path& executablePath) {
    std::error_code pathError;
    const auto absoluteExecutablePath =
        std::filesystem::absolute(executablePath, pathError);
    std::filesystem::path logDirectory;
    if (pathError) {
        pathError.clear();
        logDirectory = std::filesystem::current_path(pathError);
    } else {
        logDirectory = absoluteExecutablePath.parent_path();
    }
    if (pathError) {
        std::fprintf(stderr, "Unable to determine log file location: %s\n",
                     pathError.message().c_str());
        return;
    }

    const auto logPath = logDirectory / "NEE.log";
    try {
        auto consoleSink =
            std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            logPath.string(), true);
        auto logger = std::make_shared<spdlog::logger>(
            "nee", spdlog::sinks_init_list{consoleSink, fileSink});
        logger->flush_on(spdlog::level::err);
        spdlog::set_default_logger(std::move(logger));
        spdlog::info("Logging to '{}'", logPath.string());
    } catch (const spdlog::spdlog_ex& error) {
        std::fprintf(stderr, "Unable to create log file '%s': %s\n",
                     logPath.string().c_str(), error.what());
    }
}

}  // namespace

void Application::CalcFPS() {
    double const currentTime = glfwGetTime();
    ++frameCount;
    if (currentTime - lastTime >= 1.0) {
        fps = static_cast<float>(static_cast<double>(frameCount) /
                                 (currentTime - lastTime));
        lastTime += 1.0;
        frameCount = 0;
    }
}

void Application::ErrorCallback(int error, const char* description) {
    spdlog::error("GLFW Error {}: {}", error, description);
}
void Application::KeyCallback(GLFWwindow* window, int key, int /*scancode*/,
                              int action, int /*mods*/) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) {
        app->OnKey(*window, key, action);
    }
}

void Application::OnKey(GLFWwindow& window, int key, int action) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(&window, GL_TRUE);
    }
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        const auto spotLights = scene.GetSpotLights();
        if (!spotLights.empty()) {
            SpotLight& spotLight = spotLights.front();
            glm::vec3 target = spotLight.GetTarget();
            if (key == GLFW_KEY_I) target.x += 0.1F;
            if (key == GLFW_KEY_K) target.x -= 0.1F;
            if (key == GLFW_KEY_O) target.y += 0.1F;
            if (key == GLFW_KEY_L) target.y -= 0.1F;
            if (key == GLFW_KEY_U) target.z += 0.1F;
            if (key == GLFW_KEY_J) target.z -= 0.1F;
            spotLight.SetTarget(target);
        }
        if (key == GLFW_KEY_F5) {
            renderMode = GetNextRenderMode(renderMode);
        }
        if (key == GLFW_KEY_F12) {
            const std::time_t rawTime = std::time(nullptr);
            if (rawTime == kInvalidTime) {
                spdlog::error("Unable to get the current time for screenshot");
                return;
            }
            std::tm timeInfo{};
#ifdef _WIN32
            if (localtime_s(&timeInfo, &rawTime) != 0) {
#else
            if (localtime_r(&rawTime, &timeInfo) == nullptr) {
#endif
                spdlog::error("Unable to format the screenshot timestamp");
                return;
            }
            std::array<char, kScreenshotFilenameCapacity> buffer{};
            if (std::strftime(buffer.data(), buffer.size(),
                              "screenshots/Screenshot %Y-%m-%d %I.%M.%S.jpg",
                              &timeInfo) == 0) {
                spdlog::error("Screenshot filename is too long");
                return;
            }

            int result = 0;

            // allocate RAM for screen pixels (RGB)
            int const channels = 3;
            if (width <= 0 || height <= 0) {
                spdlog::error(
                    "Cannot capture a screenshot with empty dimensions");
                return;
            }
            const std::size_t pixelCount = static_cast<std::size_t>(width) *
                                           static_cast<std::size_t>(height) *
                                           static_cast<std::size_t>(channels);
            std::vector<uint8_t> pixels(pixelCount);

            // Ensure pixel data is tightly packed to avoid row padding
            glPixelStorei(GL_PACK_ALIGNMENT, 1);

            // read pixels from current buffer into RAM
            glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
                         pixels.data());

            //  tell stb to flip y
            stbi_flip_vertically_on_write(1);

            // sae as tga
            result = stbi_write_jpg(buffer.data(), width, height, channels,
                                    pixels.data(), 100);

            // stbi_write_tga returns 1 on success and 0 on error
            if (result == 0) {
                // Log STB write failure with spdlog
                spdlog::error(
                    "[STB Write Error] Could not save screenshot to: {}",
                    buffer.data());
            } else {
                {
                    // Inform about successful screenshot via spdlog
                    spdlog::info("Screenshot saved as {}", buffer.data());
                }
            }
        } else {
            {
                pGameCamera.OnKeyboard(key);
            }
        }
    }
}
void Application::MousePosCallBack(GLFWwindow* window, double x, double y) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) {
        app->OnMousePos(x, y);
    }
}

void Application::OnMousePos(double x, double y) {
    mouse.Update(x, y);
    pGameCamera.OnMouse(static_cast<int>(mouse.posX),
                        static_cast<int>(mouse.posY));
}

void Application::MouseButtonCallback(GLFWwindow* window, int button,
                                      int action, int /*mods*/) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) {
        app->OnMouseButton(button, action);
    }
}

void Application::OnMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_2)  // right button
    {
        mouse.rightButtonPressed = action == GLFW_PRESS;
    }
}

void Application::FrameBufferSizeCallback(GLFWwindow* window, int w, int h) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) {
        app->OnFrameBufferSize(w, h);
    }
}

void Application::OnFrameBufferSize(int w, int h) {
    width = w;
    height = h;
    if (gBuffer1 != nullptr) {
        gBuffer1->Resize(w, h);
    }
    pGameCamera.OnViewportResize(width, height);
    glViewport(0, 0, width, height);
}

void Application::RenderScene() {
    animationTime_ += 0.021F;
    renderQueue.Clear();
    scene.SetAnimatedYaw(30.0F * sinf(animationTime_));
    scene.Submit(renderQueue);
    const SceneView sceneView = scene.CreateView(pGameCamera, renderQueue);
    switch (renderMode) {
        case RenderMode::ForwardRendering:
            forwardPipeline->Render(sceneView);
            break;
        case RenderMode::DeferredShading:
            deferredPipeline->Render(sceneView, *forwardPipeline);
            break;
        default: {
            const GBufferDebugView debugView = [this] {
                switch (renderMode) {
                    case RenderMode::PositionBuffer:
                        return GBufferDebugView::Position;
                    case RenderMode::DiffuseBuffer:
                        return GBufferDebugView::Diffuse;
                    case RenderMode::NormalBuffer:
                        return GBufferDebugView::Normal;
                    case RenderMode::TextureCoordinateBuffer:
                        return GBufferDebugView::TextureCoordinates;
                    default:
                        return GBufferDebugView::Position;
                }
            }();
            deferredPipeline->RenderDebug(sceneView, debugView);
            break;
        }
    }
    InterfacePass();
}

void Application::InterfacePass() {
    renderQueue.ClearText();
    renderQueue.ClearDebugLines();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderQueue.SubmitDebugLine({.start = glm::vec3(1.0F, 0.0F, 0.0F),
                                 .end = glm::vec3(0.0F),
                                 .color = glm::vec3(1.0F, 0.0F, 0.0F)});
    renderQueue.SubmitDebugLine({.start = glm::vec3(0.0F, 1.0F, 0.0F),
                                 .end = glm::vec3(0.0F),
                                 .color = glm::vec3(0.0F, 1.0F, 0.0F)});
    renderQueue.SubmitDebugLine({.start = glm::vec3(0.0F, 0.0F, 1.0F),
                                 .end = glm::vec3(0.0F),
                                 .color = glm::vec3(0.0F, 0.0F, 1.0F)});
    const auto directionalLights = scene.GetDirectionalLights();
    if (!directionalLights.empty()) {
        DirectionalLight& light = directionalLights.front();
        renderQueue.SubmitDebugLine({.start = glm::vec3(0.0F),
                                     .end = light.GetDirection(),
                                     .color = light.GetColor()});
    }
    debugRenderer->Render(renderQueue.GetDebugLines(), pGameCamera);
    std::string const strCampos =
        ConvertToString(pGameCamera.GetPos().x) + "; " +
        ConvertToString(pGameCamera.GetPos().y) + "; " +
        ConvertToString(pGameCamera.GetPos().z);
    fLine1->SetText(strCampos);
    fLine1->SetPosition(-1.0F, 0.0F);
    fLine1->SetFontSize(24.0F);
    renderQueue.SubmitText(*fLine1);
    CalcFPS();
    fLine1->SetText(ConvertToString(fps));
    fLine1->SetPosition(-1.0F, 0.9F);
    fLine1->SetFontSize(24.0F);
    renderQueue.SubmitText(*fLine1);

    fLine1->SetText(std::string{GetRenderModeDescription(renderMode)});
    fLine1->SetPosition(-1.0F, -0.2F);
    fLine1->SetFontSize(36.0F);
    renderQueue.SubmitText(*fLine1);

    textRenderer->Render(renderQueue.GetText(), pGameCamera);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}
void Application::PreInitScene(GLFWwindow& window) {
    animationTime_ = 0.0F;
    lastTime = glfwGetTime();
    frameCount = 0;

    // text shader
    {
        textShader =
            CreateShader({{"shaders/text2d.vsh", ShaderType::Vertex},
                          {"shaders/text2d.fsh", ShaderType::Fragment}});
    }
    // Use smart pointer for automatic lifetime management
    interfaceFont = std::make_shared<Font2d>();
    if (!interfaceFont->Load("fonts/MagistralIC_UTF-8.fnt")) {
        return;
    }
    textRenderer =
        std::make_unique<TextRenderer>(gsl_lite::not_null{textShader});
    if (!textRenderer->RegisterFont(*interfaceFont)) {
        return;
    }
    fLine1 = std::make_unique<FontLine2d>();
    fLine1->SetFont(interfaceFont);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    fLine1->SetText("Loading...");
    fLine1->SetPosition(-1.0F, -0.1F);
    fLine1->SetFontSize(72.0F);
    textRenderer->Render(*fLine1, pGameCamera);
    glfwSwapBuffers(&window);
    initialized = false;
}

void Application::InitRender(GLFWwindow& window, std::string message) {
    if (!initialized) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CalcFPS();
        fLine1->SetText(ConvertToString(fps));
        fLine1->SetPosition(-1.0F, 0.9F);
        fLine1->SetFontSize(24.0F);
        textRenderer->Render(*fLine1, pGameCamera);

        fLine1->SetText(std::move(message));
        fLine1->SetPosition(-1.0F, -0.1F);
        fLine1->SetFontSize(36.0F);
        textRenderer->Render(*fLine1, pGameCamera);

        glfwSwapBuffers(&window);
        glfwPollEvents();
    }
}

int Application::InitScene(GLFWwindow& window) {
    initialized = false;
    glfwMakeContextCurrent(&window);
    // Pass the address loading function from GLFW
    if (gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress)) == 0) {
        return 1;
    }

    {
        meshShader = CreateShader(
            {{"shaders/vertexShader.vsh", ShaderType::Vertex},
             {"shaders/fragmentShader.fsh", ShaderType::Fragment}});
    }
    forwardPipeline =
        std::make_unique<ForwardPipeline>(gsl_lite::not_null{meshShader});

    debugShader = CreateShader({{"shaders/lightVS.vsh", ShaderType::Vertex},
                                {"shaders/lightFS.fsh", ShaderType::Fragment}});
    debugRenderer =
        std::make_unique<DebugRenderer>(gsl_lite::not_null{debugShader});

    // main material
    {
        InitRender(window, "Main material loading...");
        mainMaterial =
            std::make_shared<Material>(gsl_lite::not_null{meshShader});
    }

    const MeshHandle transparentTeapot = scene.AddMesh(
        Mesh(gsl_lite::not_null{mainMaterial}, "models/teapot.obj"));
    const MeshHandle sphere = scene.AddMesh(
        Mesh(gsl_lite::not_null{mainMaterial}, "models/normal_geosphere.obj"));
    const MeshHandle plane = scene.AddMesh(
        Mesh(gsl_lite::not_null{mainMaterial}, "models/normal_plane.obj"));

    // StencilPass material shader
    {
        InitRender(window, "StencilPass shader loading...");
        // load shader
        {
            DSStencilPassShader = CreateShader(
                {{"shaders/DSStencilPass.vsh", ShaderType::Vertex}});
        }
        // load material
        {
            DSStencilPassMaterial = std::make_shared<Material>(
                gsl_lite::not_null{DSStencilPassShader});
        }
    }

    // GBuffer shader and buffer, material
    {
        InitRender(window, "Gbuffer shader loading...");
        gBuffer1 = std::make_shared<GBuffer>(width, height);
        // load shader
        {
            DSGeometryPassShader = CreateShader(
                {{"shaders/DSGeometryPass.vsh", ShaderType::Vertex},
                 {"shaders/DSGeometryPass.fsh", ShaderType::Fragment}});
        }
        // load material
        {
            DSGeometryPassMaterial = std::make_shared<Material>(
                gsl_lite::not_null{DSGeometryPassShader});
        }
    }

    // light shaders
    {
        InitRender(window, "Light shader loading...");
        // point light shader
        {  // load shader
            {
                DSPointLightShader = CreateShader(
                    {{"shaders/DSPointLight.vsh", ShaderType::Vertex},
                     {"shaders/DSPointLight.fsh", ShaderType::Fragment}});
            }
            // load material
            {
                DSPointLightMaterial = std::make_shared<Material>(
                    gsl_lite::not_null{DSPointLightShader});
            }
        }

        // directional light shader
        {  // load shader
            {
                DSDirectionalLightShader = CreateShader(
                    {{"shaders/DSDirectionalLight.vsh", ShaderType::Vertex},
                     {"shaders/DSDirectionalLight.fsh", ShaderType::Fragment}});
            }
            // load material
            {
                DSDirectionalLightMaterial = std::make_shared<Material>(
                    gsl_lite::not_null{DSDirectionalLightShader});
            }
        }

        // spotlight shader
        {
            // load shader
            {
                DSSpotLightShader = CreateShader(
                    {{"shaders/DSSpotLight.vsh", ShaderType::Vertex},
                     {"shaders/DSSpotLight.fsh", ShaderType::Fragment}});
            }
            // load material
            {
                DSSpotLightMaterial = std::make_shared<Material>(
                    gsl_lite::not_null{DSSpotLightShader});
            }
        }
    }

    // skybox shader
    {
        InitRender(window, "Skybox shader loading...");

        skyboxShader =
            CreateShader({{"shaders/skybox.vsh", ShaderType::Vertex},
                          {"shaders/skybox.fsh", ShaderType::Fragment}});
    }

    // The viewer faces -Z by default, so the front image belongs on -Z.
    scene.SetSkybox(SkyBox(gsl_lite::not_null{skyboxShader}, "textures",
                           "sp3right.tga", "sp3left.tga", "sp3top.tga",
                           "sp3bot.tga", "sp3back.tga", "sp3front.tga"));
    // setup lights and unit vectors
    {
        InitRender(window, "Init lights...");
        scene.AddDirectionalLight(DirectionalLight(-1.5F, -1.0F,
                                                   -1.5F,  // direction
                                                   0.1F, 0.1F, 0.1F));
        scene.AddPointLight(PointLight(0, 1, -0.2,     // position
                                       1.0, 1.0, 1.0,  // color
                                       1.0));
        scene.AddPointLight(PointLight(1.5, 0.4, 0.0,   // position
                                       0.3, 0.3, 1.0,   // color
                                       1.1));           // power
        scene.AddSpotLight(SpotLight(1.5F, 0.0F, 0.5F,  // target
                                     1.0F, 0.4F, 0.4F,  // color
                                     0.0F, 1.0F, 0.0F,  // position
                                     35.0F));           // cutoff in degrees
    }

    // other
    {
        InitRender(window, "Final steps...");
        scene.AddBillboard(Billboard("textures/monster_hellknight.png"),
                           glm::vec3(0.0F));

        scene.AddMeshInstance(
            plane,
            Transform{.position = glm::vec3(0.0F, -3.0F, 0.0F),
                      .rotationDegrees = glm::vec3(0.0F),
                      .scale = glm::vec3(30.0F)},
            mainMaterial);
        const PerlinNoise noise(1, 10.3, 0.5, 2, 42);
        for (int row = -kGridExtentSteps; row < kGridExtentSteps; ++row) {
            const float i = static_cast<float>(row) / kGridStepsPerUnit;
            for (int column = -kGridExtentSteps; column < kGridExtentSteps;
                 ++column) {
                const float j = static_cast<float>(column) / kGridStepsPerUnit;
                scene.AddMeshInstance(
                    sphere,
                    Transform{
                        .position = glm::vec3(
                            i, static_cast<float>(noise.GetHeight(i, j)), j),
                        .rotationDegrees = glm::vec3(0.0F),
                        .scale = glm::vec3(0.05F)},
                    mainMaterial, RenderLayer::Opaque, 1.0F, true);
            }
        }
        scene.AddMeshInstance(
            transparentTeapot,
            Transform{.position = glm::vec3(0.0F, -1.0F, -3.0F),
                      .rotationDegrees = glm::vec3(0.0F),
                      .scale = glm::vec3(0.05F)},
            mainMaterial, RenderLayer::Transparent, 0.45F, true);
        // fLine1->SetAspectRatio(width,height);
    }
    deferredPipeline =
        std::make_unique<DeferredPipeline>(DeferredPipelineResources{
            .gBuffer = gsl_lite::not_null{gBuffer1},
            .geometryShader = gsl_lite::not_null{DSGeometryPassShader},
            .stencilShader = gsl_lite::not_null{DSStencilPassShader},
            .pointLightShader = gsl_lite::not_null{DSPointLightShader},
            .spotLightShader = gsl_lite::not_null{DSSpotLightShader},
            .directionalLightShader =
                gsl_lite::not_null{DSDirectionalLightShader},
            .geometryMaterial = gsl_lite::not_null{DSGeometryPassMaterial},
            .stencilMaterial = gsl_lite::not_null{DSStencilPassMaterial},
            .pointLightMaterial = gsl_lite::not_null{DSPointLightMaterial},
            .spotLightMaterial = gsl_lite::not_null{DSSpotLightMaterial},
            .directionalLightMaterial =
                gsl_lite::not_null{DSDirectionalLightMaterial}});
    glFlush();
    initialized = true;
    return 0;
}

int Application::Run(GLFWwindow& window) {
    glfwSetWindowUserPointer(&window, this);
    glfwSetFramebufferSizeCallback(&window, &FrameBufferSizeCallback);
    glfwSetKeyCallback(&window, &KeyCallback);
    glfwSetCursorPosCallback(&window, &MousePosCallBack);
    glfwSetMouseButtonCallback(&window, &MouseButtonCallback);
    glfwMakeContextCurrent(&window);

    int result = EXIT_SUCCESS;
    {
        glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
        glEnable(GL_CULL_FACE);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, width, height);

        PreInitScene(window);
        if (InitScene(window) != 0) {
            result = EXIT_FAILURE;
        } else {
            while (glfwWindowShouldClose(&window) == 0) {
                RenderScene();
                glfwSwapBuffers(&window);
                glfwPollEvents();
            }
        }
    }

    glfwSetWindowUserPointer(&window, nullptr);
    return result;
}

int main(int argc, char* argv[]) {
    ConfigureLogging(argc > 0 ? std::filesystem::path(argv[0])
                              : std::filesystem::path("NEE"));
    glfwSetErrorCallback(&Application::ErrorCallback);
    if (glfwInit() == 0) {
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    // GLFW's default 1.0 request attempts the highest available backwards-
    // compatible context. The renderers do not yet bind VAOs, so do not
    // request a core profile here.

    std::string title("HOGL ");
    title += AutoVersion::STATUS;
    title += " ";
    title += static_cast<const char*>(AutoVersion::FULLVERSION_STRING);
    title += " build ";
    title += ConvertToString(AutoVersion::BUILDS_COUNT);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    if (gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress)) == 0) {
        spdlog::critical("Failed to initialize GLAD");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    const int glMajor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    const int glMinor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    const int profile = glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE);
    // OpenGL returns null-terminated byte strings through GLubyte pointers.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto* version =
        reinterpret_cast<const char*>(glGetString(GL_VERSION));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto* renderer =
        reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* profileName = "unspecified";
    if (profile == GLFW_OPENGL_COMPAT_PROFILE) {
        profileName = "compatibility";
    } else if (profile == GLFW_OPENGL_CORE_PROFILE) {
        profileName = "core";
    }
    spdlog::info("OpenGL {}.{} [{}] ({}), renderer: {}", glMajor, glMinor,
                 profileName, version != nullptr ? version : "unknown",
                 renderer != nullptr ? renderer : "unknown");
    if (glMajor < 3 || (glMajor == 3 && glMinor < 3)) {
        spdlog::critical("OpenGL 3.3 or newer is required by the shaders");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    if (profile == GLFW_OPENGL_CORE_PROFILE) {
        spdlog::critical(
            "This renderer requires an OpenGL compatibility context");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    int result = 0;
    {
        Application app;
        result = app.Run(*window);
    }  // Destroy GL resources while the window's context still exists.
    glfwDestroyWindow(window);
    glfwTerminate();
    spdlog::shutdown();
    return result;
}
