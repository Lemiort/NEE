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

void ConfigureLogging(const char* executablePath) {
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

// The scene passes historically use a 30 degree, 1..1000 projection even
// when the interactive camera has different clipping settings.
glm::mat4 RenderPassViewProjection(const Camera& camera) {
    const glm::mat4 projection =
        glm::perspectiveRH_NO(glm::radians(30.0F),
                              static_cast<float>(camera.GetWidth()) /
                                  static_cast<float>(camera.GetHeight()),
                              1.0F, 1000.0F);
    return projection * camera.GetViewMatrix();
}

void SetLightColor(GLint uniformLocation, const std::array<GLfloat, 3>& color) {
    const glm::mat4 colorMatrix = glm::scale(
        glm::mat4(1.0F), glm::vec3(color.at(0), color.at(1), color.at(2)));
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE,
                       glm::value_ptr(colorMatrix));
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
        app->OnKey(window, key, action);
    }
}

void Application::OnKey(GLFWwindow* window, int key, int action) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_I) {
            spotLight1->target.at(0) += 0.1F;
        }
        if (key == GLFW_KEY_K) {
            spotLight1->target.at(0) -= 0.1F;
        }
        if (key == GLFW_KEY_O) {
            spotLight1->target.at(1) += 0.1F;
        }
        if (key == GLFW_KEY_L) {
            spotLight1->target.at(1) -= 0.1F;
        }
        if (key == GLFW_KEY_U) {
            spotLight1->target.at(2) += 0.1F;
        }
        if (key == GLFW_KEY_J) {
            spotLight1->target.at(2) -= 0.1F;
        }
        if (key == GLFW_KEY_F5) {
            renderType = static_cast<RenderType>(
                (std::to_underlying(renderType) + 1) % 6);
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
    if (smfbo1 != nullptr) {
        smfbo1->Init(w, h);
    }
    if (gBuffer1 != nullptr) {
        gBuffer1->Init(w, h);
    }
    pGameCamera.OnViewportResize(width, height);
    glViewport(0, 0, width, height);
}

void Application::ShadowPass() {
    smfbo1->BindForWriting();
    // gBuffer1->BindForWriting();
    // Drawing stage
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // CalcFPS();
    Scale += 0.021F;
    TestMesh.SetRotation(0, 30 * sinf(Scale), 0);
    TestMesh.SetScale(0.02, 0.02, 0.02);
    TestMesh.SetPosition(0, -0.2, 0);
    // light3->SetPos(pGameCamera.GetPos());
    // light3->SetDir(pGameCamera.GetTarget()-pGameCamera.GetPos());
    Camera const lightCam(width, height, 45, 1, 1000.0F, spotLight1->GetPos(),
                          glm::vec3(-1.0F, -1.0F, -1.0F),
                          glm::vec3(0.0F, 1.0F, 0.0F));

    Plane.SetScale(30.0F, 30.0F, 30.0F);
    Plane.SetPosition(0.0F, -3.0F, 0.0F);
    Plane.SetRotation(0.0, 0.0, 0.0);
    const glm::mat4 lightViewProjection = RenderPassViewProjection(lightCam);

    shadowShader->Use();
    gCamViewID = shadowShader->GetUniformLocation("gVC");
    rotateID = shadowShader->GetUniformLocation("mRotate");
    camPosID = shadowShader->GetUniformLocation("s_vCamPos");

    glUniformMatrix4fv(gCamViewID, 1, GL_FALSE,
                       glm::value_ptr(lightViewProjection));
    // Lighting
    {
        // Directional light
        SetLightColor(dirLightColID, directionalLight1->color);
        glUniform3f(dirLightDirID, directionalLight1->direction.at(0),
                    directionalLight1->direction.at(1),
                    directionalLight1->direction.at(2));
        // point light
        SetLightColor(pointLightColID, pointLight1->color);
        const glm::vec3 pointLightPosition = pointLight1->GetPosition();
        glUniform3f(pointLightPosID, pointLightPosition.x, pointLightPosition.y,
                    pointLightPosition.z);
        glUniform1f(pointLightIntID, pointLight1->power);

        // spot light
        SetLightColor(spotLightColID, spotLight1->color);
        glUniform3f(spotLightDirID, spotLight1->direction.at(0),
                    spotLight1->direction.at(1), spotLight1->direction.at(2));
        glUniform1f(spotLightCutoffID,
                    glm::cos(glm::radians(spotLight1->Cutoff)));
        const glm::vec3 spotLightPosition = spotLight1->GetPosition();
        glUniform3f(spotLightPosID, spotLightPosition.x, spotLightPosition.y,
                    spotLightPosition.z);
    }

    // camera rotation for specular
    glUniform3f(camPosID, lightCam.GetPos().x, lightCam.GetPos().y,
                lightCam.GetPos().z);

    Sphere.SetMaterial(shadowMaterial);
    for (int row = -kGridExtentSteps; row < kGridExtentSteps; ++row) {
        const float i = static_cast<float>(row) / kGridStepsPerUnit;
        for (int column = -kGridExtentSteps; column < kGridExtentSteps;
             ++column) {
            const float j = static_cast<float>(column) / kGridStepsPerUnit;
            Sphere.SetScale(0.05F, 0.05F, 0.05F);
            Sphere.SetRotation(0, 30.0F * sinf(Scale), 0);
            Sphere.SetPosition(i, static_cast<float>(noise1->GetHeight(i, j)),
                               j);
            Sphere.Render(lightCam);
        }
    }
    Plane.SetMaterial(shadowMaterial);
    Plane.Render(lightCam);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::RenderPass() {
    // Drawing stage
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // CalcFPS();
    Scale += 0.021F;
    TestMesh.SetRotation(0, 30 * sinf(Scale), 0);
    TestMesh.SetScale(0.02, 0.02, 0.02);
    TestMesh.SetPosition(0, -0.2, 0);

    Plane.SetScale(30.0F, 30.0F, 30.0F);
    Plane.SetPosition(0.0F, -3.0F, 0.0F);
    Plane.SetRotation(0.0, 0.0, 0.0);
    const glm::mat4 viewProjection = RenderPassViewProjection(pGameCamera);

    Camera const lightCam(width, height, pGameCamera.GetFov(),
                          pGameCamera.GetZNear(), pGameCamera.GetZFar(),
                          spotLight1->GetPos(), glm::vec3(-1.0, -1.0, -1.0),
                          glm::vec3(0.0, 1.0, 0.0));

    const glm::mat4 lightViewProjection = RenderPassViewProjection(lightCam);

    /*Sphere.SetMaterial(mainMaterial);
    for(float i=-5.0f;i<5.0f;i+=0.1f)
        for(float j=-5.0f;j<5.0f;j+=0.1f)
    {
        Sphere.SetScale(0.001f,0.001f,0.001f);
        Sphere.SetRotate(0,30*sinf(Scale),0);
        Sphere.SetPosition(i,noise1->GetHeight(i,j),j);
        Sphere.Render(30,width, height, 1, 1000,pGameCamera);
    }*/

    /* TestMesh.Render(30,width, height, 1, 1000,pGameCamera);
      TestMesh.SetPosition(-3,-0.2,0);
      TestMesh.Render(30,width, height, 1, 1000,pGameCamera);*/
    skybox1->Render(pGameCamera);

    /*Texture2D* tempTexture=new Texture2D(gBuffer1->GetTexture(2),false);
    //shadowMeshMaterial->SetShadowTexture(tempTexture);
    mainMaterial->SetColorTexture(tempTexture);
    fLine1->Render(tempTexture->GetParameters(),-1.0f,-0.2f,32.0f);*/
    Plane.SetMaterial(mainMaterial);
    // Simple copy-paste
    {
        meshShader->Use();
        GLint const gLightCamViewID =
            meshShader->GetUniformLocation("gLightVC");
        gCamViewID = meshShader->GetUniformLocation("gVC");
        rotateID = meshShader->GetUniformLocation("mRotate");
        camPosID = meshShader->GetUniformLocation("s_vCamPos");

        glUniformMatrix4fv(gCamViewID, 1, GL_FALSE,
                           glm::value_ptr(viewProjection));
        glUniformMatrix4fv(gLightCamViewID, 1, GL_FALSE,
                           glm::value_ptr(lightViewProjection));

        // Lighting
        {
            // Directional light
            SetLightColor(dirLightColID, directionalLight1->color);
            glUniform3f(dirLightDirID, directionalLight1->direction.at(0),
                        directionalLight1->direction.at(1),
                        directionalLight1->direction.at(2));
            // Point light
            SetLightColor(pointLightColID, pointLight1->color);
            const glm::vec3 pointLightPosition = pointLight1->GetPosition();
            glUniform3f(pointLightPosID, pointLightPosition.x,
                        pointLightPosition.y, pointLightPosition.z);
            glUniform1f(pointLightIntID, pointLight1->power);
            // spot light
            SetLightColor(spotLightColID, spotLight1->color);
            glUniform3f(spotLightDirID, spotLight1->direction.at(0),
                        spotLight1->direction.at(1),
                        spotLight1->direction.at(2));
            glUniform1f(spotLightCutoffID,
                        glm::cos(glm::radians(spotLight1->Cutoff)));
            const glm::vec3 spotLightPosition = spotLight1->GetPosition();
            glUniform3f(spotLightPosID, spotLightPosition.x,
                        spotLightPosition.y, spotLightPosition.z);
        }

        // camera rotation for specular
        glUniform3f(camPosID, pGameCamera.GetPos().x, pGameCamera.GetPos().y,
                    pGameCamera.GetPos().z);
    }
    // Plane.SetMaterial(mainMaterial);
    Plane.Render(pGameCamera);
    Sphere.SetMaterial(mainMaterial);
    for (int row = -kGridExtentSteps; row < kGridExtentSteps; ++row) {
        const float i = static_cast<float>(row) / kGridStepsPerUnit;
        for (int column = -kGridExtentSteps; column < kGridExtentSteps;
             ++column) {
            const float j = static_cast<float>(column) / kGridStepsPerUnit;
            Sphere.SetScale(0.05F, 0.05F, 0.05F);
            Sphere.SetRotation(0, 30.0F * sinf(Scale), 0);
            Sphere.SetPosition(i, static_cast<float>(noise1->GetHeight(i, j)),
                               j);
            Sphere.Render(pGameCamera);
        }
    }

    bb1->Render(pGameCamera);

    // xline->Render(pGameCamera,width, height);
    //  yline->Render(pGameCamera, width, height);
    //  zline->Render(pGameCamera,width, height);

    //
    // tline2->Render(-1,0.9,64.0,(char*)(ConvertToString(Sphere.GetNumFaces()*10000)+"
    // faces").c_str());
    // fLine1->Render(ConvertToString(fps).c_str(),-1.0f,0.9f,30.0f);
    //  spfaces=Sphere.GetNumFaces()*100*100+TestMesh.GetNumFaces()+Plane.GetNumFaces();
    //  fLine1->Render((ConvertToString(spfaces)+"faces").c_str(),-1.0f,0.0f,45.0f);*/
}

void Application::DSBeginLightPasses() {
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // gBuffer1->BindForReading();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Application::DSEndLigtPasses() { glDisable(GL_BLEND); }

void Application::DSLightingPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gBuffer1->BindForReading();

    // GLsizei HalfWidth = (GLsizei)(width / 2.0f);
    // GLsizei HalfHeight = (GLsizei)(height / 2.0f);

    // gBuffer1->CheckTextures();
    /*
    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
    glBlitFramebuffer(0, 0,width, height, 0, 0, HalfWidth, HalfHeight,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, width, height, 0, HalfHeight, HalfWidth, height,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, width, height, HalfWidth, HalfHeight, width, height,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);

    gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
    glBlitFramebuffer(0, 0, width, height, HalfWidth, 0, width, HalfHeight,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);*/
    switch (renderType) {
        case RenderType::POSITION_BUFFER:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case RenderType::DIFFUSE_BUFFER:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case RenderType::NORMAL_BUFFER:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case RenderType::TEXCOORD_BUFFER:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        default:
            // dont bind for regular rendering
            break;
    }

    /* fLine1->Render(ConvertToString(fps).c_str(),-1.0f,0.9f,30.0f);
     spfaces=Sphere.GetNumFaces()*100*100+TestMesh.GetNumFaces()+Plane.GetNumFaces();
     fLine1->Render((ConvertToString(spfaces)+"
     faces").c_str(),-1.0f,0.0f,45.0f);*/
}

void Application::DSStencilPass(Light& light) {
    // m_nullTech.Enable();
    // enable shader
    DSStencilPassShader->Use();

    // Disable color / depth writing and enable stencil
    gBuffer1->BindForStencilPass();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  //??

    glDisable(GL_CULL_FACE);

    glClear(GL_STENCIL_BUFFER_BIT);

    // we need stencil test, but we want it to always pass. Only depth test is
    // important.
    glStencilFunc(GL_ALWAYS, 0, 0);

    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);

    const glm::mat4 viewProjection = RenderPassViewProjection(pGameCamera);

    // define camera variable addresses
    gCamViewID = DSStencilPassShader->GetUniformLocation("gVC");
    rotateID = DSStencilPassShader->GetUniformLocation("mRotate");
    camPosID = DSStencilPassShader->GetUniformLocation("s_vCamPos");

    // load the camera matrix
    glUniformMatrix4fv(gCamViewID, 1, GL_FALSE, glm::value_ptr(viewProjection));

    light.SetMaterial(DSStencilPassMaterial);
    light.Render(pGameCamera);
}

void Application::DSPointLightPass(PointLight& pointLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    const glm::mat4 viewProjection = RenderPassViewProjection(pGameCamera);

    // enable shader
    DSPointLightShader->Use();

    // define camera variable addresses
    gCamViewID = DSPointLightShader->GetUniformLocation("gVC");
    rotateID = DSPointLightShader->GetUniformLocation("mRotate");
    camPosID = DSPointLightShader->GetUniformLocation("s_vCamPos");

    // load the camera matrix
    glUniformMatrix4fv(gCamViewID, 1, GL_FALSE, glm::value_ptr(viewProjection));
    // load the camera rotation for specular
    glUniform3f(camPosID, pGameCamera.GetPos().x, pGameCamera.GetPos().y,
                pGameCamera.GetPos().z);

    // load textures into shader
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(0), 4);  // world pos
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(1), 5);  // diffuse
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(2), 6);  // normal
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(3), 7);  // UV
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(4), 8);  // specular

    // load buffer data
    //  glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //  glClear(/*GL_COLOR_BUFFER_BIT |*/GL_DEPTH_BUFFER_BIT);

    // Define light parameters address
    pointLightPosID =
        DSPointLightShader->GetUniformLocation("s_vPointLightPos");
    pointLightIntID =
        DSPointLightShader->GetUniformLocation("pointLightIntensity");
    pointLightColID = DSPointLightShader->GetUniformLocation("pointLightColor");

    // load light parameters for source 1
    SetLightColor(pointLightColID, pointLight.color);
    const glm::vec3 pointLightPosition = pointLight.GetPosition();
    glUniform3f(pointLightPosID, pointLightPosition.x, pointLightPosition.y,
                pointLightPosition.z);
    glUniform1f(pointLightIntID, pointLight.power);

    pointLight.SetMaterial(DSPointLightMaterial);
    pointLight.Render(pGameCamera);

    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}

void Application::DSSpotLightPass(SpotLight& spotLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    const glm::mat4 viewProjection = RenderPassViewProjection(pGameCamera);

    // enable shader
    DSSpotLightShader->Use();

    // define camera variable addresses
    gCamViewID = DSSpotLightShader->GetUniformLocation("gVC");
    rotateID = DSSpotLightShader->GetUniformLocation("mRotate");
    camPosID = DSSpotLightShader->GetUniformLocation("s_vCamPos");

    // load the camera matrix
    glUniformMatrix4fv(gCamViewID, 1, GL_FALSE, glm::value_ptr(viewProjection));
    // load the camera rotation for specular
    glUniform3f(camPosID, pGameCamera.GetPos().x, pGameCamera.GetPos().y,
                pGameCamera.GetPos().z);

    // load textures into shader
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(0), 4);  // world pos
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(1), 5);  // diffuse
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(2), 6);  // normal
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(3), 7);  // UV
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(4), 8);  // specular

    // define the address of the light parameters
    spotLightPosID = DSSpotLightShader->GetUniformLocation("sLightPos");
    spotLightColID = DSSpotLightShader->GetUniformLocation("sLightCol");
    spotLightDirID = DSSpotLightShader->GetUniformLocation("sLightDir");
    spotLightCutoffID = DSSpotLightShader->GetUniformLocation("sLightCutoff");

    // Load the light parameters for source 1
    const glm::vec3 spotLightPosition = spotLight.GetPosition();
    glUniform3f(spotLightPosID, spotLightPosition.x, spotLightPosition.y,
                spotLightPosition.z);
    SetLightColor(spotLightColID, spotLight.color);
    glUniform3f(spotLightDirID, spotLight.direction.at(0),
                spotLight.direction.at(1), spotLight.direction.at(2));
    glUniform1f(spotLightCutoffID, cosf(glm::radians(spotLight.Cutoff)));

    // enable buffer data
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    spotLight.SetMaterial(DSSpotLightMaterial);
    spotLight.Render(pGameCamera);

    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
}

void Application::DSDirectionalLightPass(DirectionalLight& directionalLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();
    // glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    const glm::mat4 viewProjection = RenderPassViewProjection(pGameCamera);

    DSDirectionalLightShader->Use();
    // get camera parameter addresses
    gCamViewID = DSDirectionalLightShader->GetUniformLocation("gVC");
    rotateID = DSDirectionalLightShader->GetUniformLocation("mRotate");
    camPosID = DSDirectionalLightShader->GetUniformLocation("s_vCamPos");
    // Load the camera rotation for specular
    glUniform3f(camPosID, pGameCamera.GetPos().x, pGameCamera.GetPos().y,
                pGameCamera.GetPos().z);
    // load the camera matrix
    glUniformMatrix4fv(gCamViewID, 1, GL_FALSE, glm::value_ptr(viewProjection));

    // Get the address of light variables
    dirLightColID =
        DSDirectionalLightShader->GetUniformLocation("dirLightColor");
    dirLightDirID =
        DSDirectionalLightShader->GetUniformLocation("dirLightDirection");

    // Load the light parameters
    SetLightColor(dirLightColID, directionalLight.color);
    glUniform3f(dirLightDirID, directionalLight.direction.at(0),
                directionalLight.direction.at(1),
                directionalLight.direction.at(2));

    // Load textures into shader
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(0),
                                           4);  // world pos
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(1),
                                           5);  // diffuse
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(2),
                                           6);  // normal
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(3), 7);  // UV
    DSDirectionalLightMaterial->SetTexture(gBuffer1->GetTexture(4),
                                           8);  // specular

    directionalLight.SetMaterial(DSDirectionalLightMaterial);

    /*printf("\n %f %f %f",directionalLight1->direction[0],
                            directionalLight1->direction[1],
                            directionalLight1->direction[2]);*/
    // glUniform1f(pointLightIntID,pointLight1->power);

    // enable data from buffer
    //  glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //  glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);

    directionalLight.Render(pGameCamera);
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Application::DSFinalPass() {
    gBuffer1->BindForFinalPass();
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH,
                      WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void Application::InterfacePass() {
    renderQueue.ClearText();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glDisable(GL_DEPTH_TEST);
    xline->Render(pGameCamera);
    yline->Render(pGameCamera);
    zline->Render(pGameCamera);
    std::string const strCampos =
        ConvertToString(pGameCamera.GetPos().x) + "; " +
        ConvertToString(pGameCamera.GetPos().y) + "; " +
        ConvertToString(pGameCamera.GetPos().z);
    // fLine1->Render((strCampos).c_str(),-1.0f,0.0f,24.0f);
    fLine1->SetText(strCampos);
    fLine1->SetPosition(-1.0F, 0.0F);
    fLine1->SetFontSize(24.0F);
    renderQueue.SubmitText(*fLine1);
    // gBuffer1->CheckTextures();
    CalcFPS();
    // fLine1->Render(ConvertToString(fps),-1.0f,0.9f,24.0f);
    fLine1->SetText(ConvertToString(fps));
    fLine1->SetPosition(-1.0F, 0.9F);
    fLine1->SetFontSize(24.0F);
    renderQueue.SubmitText(*fLine1);

    if (renderType == RenderType::DEFERRED_SHADING) {
        // fLine1->Render("Deferred shading",-1.0f,-0.2f,36.0f);
        fLine1->SetText("Deferred shading");
        fLine1->SetPosition(-1.0F, -0.2F);
        fLine1->SetFontSize(36.0F);
        renderQueue.SubmitText(*fLine1);
    }

    textRenderer->Render(renderQueue.GetText(), pGameCamera);

    // pointLight1->Render(30,width, height, 1, 1000,pGameCamera);
    dirLightLine->Render(pGameCamera);
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void Application::DSGeometryPass() {
    gBuffer1->BindForGeomPass();
    // gBuffer1->BindForWriting();
    // Only the geometry pass updates the depth test
    glDepthMask(GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    CalcFPS();

    const glm::mat4 viewProjection = RenderPassViewProjection(pGameCamera);

    DSGeometryPassShader->Use();
    gCamViewID = DSGeometryPassShader->GetUniformLocation("gVC");

    glUniformMatrix4fv(gCamViewID, 1, GL_FALSE, glm::value_ptr(viewProjection));

    Sphere.SetMaterial(DSGeometryPassMaterial);
    for (int row = -kGridExtentSteps; row < kGridExtentSteps; ++row) {
        const float i = static_cast<float>(row) / kGridStepsPerUnit;
        for (int column = -kGridExtentSteps; column < kGridExtentSteps;
             ++column) {
            const float j = static_cast<float>(column) / kGridStepsPerUnit;
            Sphere.SetScale(0.05F, 0.05F, 0.05F);
            Sphere.SetRotation(0, 30.0F * sinf(Scale), 0);
            Sphere.SetPosition(i, static_cast<float>(noise1->GetHeight(i, j)),
                               j);
            Sphere.Render(pGameCamera);
        }
    }

    // glBindFramebuffer(GL_FRAMEBUFFER,0);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}

void Application::RenderScene() {
    // ShadowPass();
    // RenderPass();
    // DSGeometryPass();
    // DSBeginLightPasses(); // not needed yet
    // DSPointLightPass();
    // DSLightingPass();
    // normal deferred shading
    if (renderType == RenderType::DEFERRED_SHADING) {
        gBuffer1->StartFrame();
        DSGeometryPass();
        // To update the stencil buffer, it must be activated,
        // and it is also required during lighting passes because light
        // renders only when the stencil pass succeeds.
        glEnable(GL_STENCIL_TEST);
        // DSBeginLightPasses(); // already deprecated
        {
            DSStencilPass(*pointLight1);
            DSPointLightPass(*pointLight1);

            DSStencilPass(*pointLight2);
            DSPointLightPass(*pointLight2);

            DSStencilPass(*spotLight1);
            DSSpotLightPass(*spotLight1);
        }
        // Directed light does not require a stencil
        // as its effect is not distance limited.
        glDisable(GL_STENCIL_TEST);

        DSDirectionalLightPass(*directionalLight1);

        DSFinalPass();
        // DSEndLightPasses();
    }
    // debugging view
    else if (std::to_underlying(renderType) <=
             std::to_underlying(RenderType::TEXCOORD_BUFFER)) {
        DSGeometryPass();
        DSLightingPass();
    } else {
        RenderPass();
    }
    InterfacePass();
}
void Application::PreInitScene(GLFWwindow* window) {
    Scale = 0;
    lastTime = glfwGetTime();
    frameCount = 0;

    // text shader
    {
        textShader = std::make_shared<Shader>();
        textShader->AddShader(ReadFile("shaders/text2d.vsh"), VertexShader);
        textShader->AddShader(ReadFile("shaders/text2d.fsh"), FragmnetShader);
        textShader->Init();
    }
    // Use smart pointer for automatic lifetime management
    interfaceFont = std::make_shared<Font2d>();
    if (!interfaceFont->Load("fonts/MagistralIC_UTF-8.fnt")) {
        return;
    }
    textRenderer = std::make_unique<TextRenderer>();
    if (!textRenderer->Init(textShader) ||
        !textRenderer->RegisterFont(interfaceFont)) {
        return;
    }
    fLine1 = std::make_unique<FontLine2d>();
    fLine1->SetFont(interfaceFont);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // fLine1->Render("Loading...",-1.0f,-0.1f,72.0f);
    fLine1->SetText("Loading...");
    fLine1->SetPosition(-1.0F, -0.1F);
    fLine1->SetFontSize(72.0F);
    textRenderer->Render(*fLine1, pGameCamera);
    glfwSwapBuffers(window);
    initialized = false;
}

void Application::InitRender(GLFWwindow* window, string message) {
    // thread thr(InitScene,hiddenWindow);

    // thr.detach();
    // InitScene(window);

    // while(!glfwWindowShouldClose(window) && initialized == false)
    if (!initialized) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CalcFPS();
        // fLine1->Render(ConvertToString(fps),-1.0f,0.9f,24.0f);
        fLine1->SetText(ConvertToString(fps));
        fLine1->SetPosition(-1.0F, 0.9F);
        fLine1->SetFontSize(24.0F);
        textRenderer->Render(*fLine1, pGameCamera);

        // fLine1->Render(message,-1.0f,-0.1f,36.0f);
        fLine1->SetText(std::move(message));
        fLine1->SetPosition(-1.0F, -0.1F);
        fLine1->SetFontSize(36.0F);
        textRenderer->Render(*fLine1, pGameCamera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // thr.join();
}

int Application::InitScene(GLFWwindow* window) {
    /*std::thread thr(InitRender, window, string("test"));

    thr.detach();*/

    initialized = false;
    glfwMakeContextCurrent(window);
    // Pass the address loading function from GLFW
    if (gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress)) == 0) {
        return 1;
    }

    /*Scale=0;
    lastTime = glfwGetTime();
    frameCount=0;

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);**/

    // normal shader
    //  InitRender(window, "Normal shader loading...");

    {
        meshShader = make_shared<Shader>();
        meshShader->AddShader(ReadFile("shaders/vertexShader.vsh"),
                              VertexShader);
        meshShader->AddShader(ReadFile("shaders/fragmentShader.fsh"),
                              FragmnetShader);
        meshShader->Init();

        gCamViewID = meshShader->GetUniformLocation("gVC");
        rotateID = meshShader->GetUniformLocation("mRotate");
        camPosID = meshShader->GetUniformLocation("s_vCamPos");
    }

    // main material
    {
        InitRender(window, "Main material loading...");
        mainMaterial = make_shared<Material>();
        mainMaterial->Init(meshShader);
    }

    // Sphere.Init(mainMaterial, "models/sphere.obj");
    if (!TestMesh.Init(mainMaterial, "models/torus1.obj")) {
        spdlog::critical(
            "Scene initialization aborted: required model '{}' failed to "
            "initialize",
            "models/torus1.obj");
        return EXIT_FAILURE;
    }
    // Plane.Init(mainMaterial, "models/normal_plane.obj");

    // shadow shader
    {
        InitRender(window, "Shade shader loading...");
        shadowShader = make_shared<Shader>();
        shadowShader->AddShader(ReadFile("shaders/fbo.vsh"), VertexShader);
        shadowShader->AddShader(ReadFile("shaders/fbo.fsh"), FragmnetShader);
        shadowShader->Init();
    }

    // shadow material
    {
        InitRender(window, "Shade Material loading...");
        shadowMaterial = make_shared<Material>();
        shadowMaterial->Init(shadowShader);
    }

    // shaded shader
    {
        InitRender(window, "Shaded shader loading...");
        shadowMeshShader = make_shared<Shader>();
        shadowMeshShader->AddShader(ReadFile("shaders/shadowed.vsh"),
                                    VertexShader);
        shadowMeshShader->AddShader(ReadFile("shaders/shadowed.fsh"),
                                    FragmnetShader);
        shadowMeshShader->Init();
    }

    // shaded material
    {
        InitRender(window, "Shaded material loading...");
        shadowMeshMaterial = make_shared<Material>();
        shadowMeshMaterial->Init(shadowMeshShader);
        whiteTexture = make_shared<Texture2D>();
        whiteTexture->Load("Textures/white.png");
        shadowMeshMaterial->SetColorTexture(whiteTexture);
        if (!Sphere.Init(shadowMeshMaterial, "models/normal_geosphere.obj")) {
            spdlog::critical(
                "Scene initialization aborted: required model '{}' failed to "
                "initialize",
                "models/normal_geosphere.obj");
            return EXIT_FAILURE;
        }
        if (!Plane.Init(shadowMeshMaterial, "models/normal_plane.obj")) {
            spdlog::critical(
                "Scene initialization aborted: required model '{}' failed to "
                "initialize",
                "models/normal_plane.obj");
            return EXIT_FAILURE;
        }
    }

    // StencilPass material shader
    {
        InitRender(window, "StencilPass shader loading...");
        // load shader
        {
            DSStencilPassShader = make_shared<Shader>();
            DSStencilPassShader->AddShader(
                ReadFile("shaders/DSStencilPass.vsh"), VertexShader);
            DSStencilPassShader->Init();
        }
        // load material
        {
            DSStencilPassMaterial = make_shared<Material>();
            DSStencilPassMaterial->Init(DSStencilPassShader);
        }
    }

    // GBuffer shader and buffer, material
    {
        InitRender(window, "Gbuffer shader loading...");
        gBuffer1 = std::make_unique<GBuffer>();
        gBuffer1->Init(width, height);
        // load shader
        {
            DSGeometryPassShader = make_shared<Shader>();
            DSGeometryPassShader->AddShader(
                ReadFile("shaders/DSGeometryPass.vsh"), VertexShader);
            DSGeometryPassShader->AddShader(
                ReadFile("shaders/DSGeometryPass.fsh"), FragmnetShader);
            DSGeometryPassShader->Init();
        }
        // load material
        {
            DSGeometryPassMaterial = make_shared<Material>();
            DSGeometryPassMaterial->Init(DSGeometryPassShader);
        }
    }

    // light shaders
    {
        InitRender(window, "Light shader loading...");
        // point light shader
        {  // load shader
            {
                DSPointLightShader = make_shared<Shader>();
                DSPointLightShader->AddShader(
                    ReadFile("shaders/DSPointLight.vsh"), VertexShader);
                DSPointLightShader->AddShader(
                    ReadFile("shaders/DSPointLight.fsh"), FragmnetShader);
                DSPointLightShader->Init();
            }
            // load material
            {
                DSPointLightMaterial = make_shared<Material>();
                DSPointLightMaterial->Init(DSPointLightShader);
            }
        }

        // directional light shader
        {  // load shader
            {
                DSDirectionalLightShader = make_shared<Shader>();
                DSDirectionalLightShader->AddShader(
                    ReadFile("shaders/DSDirectionalLight.vsh"), VertexShader);
                DSDirectionalLightShader->AddShader(
                    ReadFile("shaders/DSDirectionalLight.fsh"), FragmnetShader);
                DSDirectionalLightShader->Init();
            }
            // load material
            {
                DSDirectionalLightMaterial = make_shared<Material>();
                DSDirectionalLightMaterial->Init(DSDirectionalLightShader);
            }
        }

        // spotlight shader
        {
            // load shader
            {
                DSSpotLightShader = make_shared<Shader>();
                DSSpotLightShader->AddShader(
                    ReadFile("shaders/DSSpotLight.vsh"), VertexShader);
                DSSpotLightShader->AddShader(
                    ReadFile("shaders/DSSpotLight.fsh"), FragmnetShader);
                DSSpotLightShader->Init();
            }
            // load material
            {
                DSSpotLightMaterial = make_shared<Material>();
                DSSpotLightMaterial->Init(DSSpotLightShader);
            }
        }
    }

    // skybox shader
    {
        InitRender(window, "Skybox shader loading...");

        skyboxShader = make_shared<Shader>();

        skyboxShader->AddShader(ReadFile("shaders/skybox.vsh"), VertexShader);
        skyboxShader->AddShader(ReadFile("shaders/skybox.fsh"), FragmnetShader);
        skyboxShader->Init();
    }

    skybox1 = std::make_unique<SkyBox>(skyboxShader);
    // The viewer faces -Z by default, so the front image belongs on -Z.
    if (!skybox1->Init("Textures", "sp3right.tga", "sp3left.tga", "sp3top.tga",
                       "sp3bot.tga", "sp3back.tga", "sp3front.tga")) {
        spdlog::critical(
            "Scene initialization aborted: required skybox model "
            "'models/sphere.obj' failed to initialize");
        return EXIT_FAILURE;
    }
    // setup lights and unit vectors
    {
        InitRender(window, "Init lights...");
        dirLightDirID = meshShader->GetUniformLocation("dLightDir");
        dirLightColID = meshShader->GetUniformLocation("dLightCol");
        spotLightDirID = meshShader->GetUniformLocation("sLightDir");
        spotLightColID = meshShader->GetUniformLocation("sLightCol");
        spotLightCutoffID = meshShader->GetUniformLocation("sLightCutoff");
        spotLightPosID = meshShader->GetUniformLocation("sLightPos");

        pointLightIntID = meshShader->GetUniformLocation("pLightInt");
        pointLightColID = meshShader->GetUniformLocation("pLightCol");
        pointLightPosID = meshShader->GetUniformLocation("pLightPos");

        directionalLight1 = std::make_unique<DirectionalLight>(
            -1.5F, -1.0F, -1.5F,  // direction
            0.5F, 0.5F, 0.5F,     // color
            DSDirectionalLightMaterial);
        pointLight1 = std::make_unique<PointLight>(0, 1, -0.2,     // position
                                                   1.0, 1.0, 1.0,  // color
                                                   1.0, DSPointLightMaterial);
        pointLight2 = std::make_unique<PointLight>(1.5, 0.4, 0.0,  // position
                                                   0.3, 0.3, 1.0,  // color
                                                   1.1,            // power
                                                   DSPointLightMaterial);
        spotLight1 = std::make_unique<SpotLight>(1.5F, 0.0F, 0.5F,  // target
                                                 1.0F, 0.4F, 0.4F,  // color
                                                 0.0F, 1.0F, 0.0F,  // position
                                                 35.0F,  // cutoff in degrees
                                                 DSSpotLightMaterial);
        glm::vec3 const PX(1, 0, 0);
        glm::vec3 const PY(0, 1, 0);
        glm::vec3 const PZ(0, 0, 1);
        glm::vec3 const P0(0, 0, 0);

        xline = std::make_unique<Line>(PX, P0, PX);
        yline = std::make_unique<Line>(PY, P0, PY, xline->GetShader());
        zline = std::make_unique<Line>(PZ, P0, PZ, xline->GetShader());
        dirLightLine = std::make_unique<Line>(P0, directionalLight1->GetDir(),
                                              directionalLight1->GetCol());
    }

    // other
    {
        InitRender(window, "Final steps...");
        bb1 = std::make_unique<Billboard>();
        bb1->Init("Textures/monster_hellknight.png");
        bb1->SetPos(glm::vec3(0, 0, 0));

        noise1 = std::make_unique<PerlinNoise>(1, 10.3, 0.5, 2, 42);
        // fLine1->Init(string("fonts/MagistralIC_UTF-8.fnt"),textShader);
        // fLine1->SetAspectRatio(width,height);

        //*************Shadow MAP FBO**********/
        // smfbo1 = new ShadowMapFBO();
        // smfbo1->Init(width,height);
        //************************************/
        // pGameCamera = new
        // Camera(width,height,light3->GetPos(),glm::vec3(-1.0,-1.0,-1.0),glm::vec3(0.0,1.0,0.0));
    }
    glFlush();
    initialized = true;
    return 0;
}

int Application::Run(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, &FrameBufferSizeCallback);
    glfwSetKeyCallback(window, &KeyCallback);
    glfwSetCursorPosCallback(window, &MousePosCallBack);
    glfwSetMouseButtonCallback(window, &MouseButtonCallback);
    glfwMakeContextCurrent(window);

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
            while (glfwWindowShouldClose(window) == 0) {
                RenderScene();
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
        }
    }

    glfwSetWindowUserPointer(window, nullptr);
    return result;
}

int main(int argc, char* argv[]) {
    ConfigureLogging(argc > 0 ? argv[0] : "NEE");
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
        result = app.Run(window);
    }  // Destroy GL resources while the window's context still exists.
    glfwDestroyWindow(window);
    glfwTerminate();
    spdlog::shutdown();
    return result;
}
