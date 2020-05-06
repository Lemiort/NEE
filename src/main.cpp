#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include <ctime>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_operation.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "Billboard.h"
#include "GBuffer.h"
#include "Light.h"
#include "Logger.h"
#include "Material.h"
#include "Mesh.h"
#include "PerlinNoise.h"
#include "ShadowMapFBO.h"
#include "Text_2D.h"
#include "skybox.h"

#define WINDOW_WIDTH 1366
#define WINDOW_HEIGHT 768

std::shared_ptr<Shader> meshShader;
std::shared_ptr<Shader> skyboxShader;
std::shared_ptr<Shader> textShader;
std::shared_ptr<Shader> lightShader;
std::shared_ptr<Shader> shadowShader;
std::shared_ptr<Shader> shadowMeshShader;
std::shared_ptr<Shader> DSGeometryPassShader;
std::shared_ptr<Shader> DSPointLightShader;
std::shared_ptr<Shader> DSDirectionalLightShader;
std::shared_ptr<Shader> DSSpotLightShader;
std::shared_ptr<Shader> DSStencilPassShader;

std::shared_ptr<Material> shadowMeshMaterial;
std::shared_ptr<Material> DSGeometryPassMaterial;
std::shared_ptr<Material> DSPointLightMaterial;
std::shared_ptr<Material> DSDirectionalLightMaterial;
std::shared_ptr<Material> DSSpotLightMaterial;
std::shared_ptr<Material> DSStencilPassMaterial;

GBuffer* gBuffer1;
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
void InitRender(GLFWwindow* window, std::string message);
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

GLuint model_id, gCamViewID;
GLuint rotation_id;
GLuint dirLightDirID, dirLightColID;
GLuint pointLightColID, pointLightIntID, pointLightPosID;
GLuint spotLightColID, spotLightDirID, spotLightCutoffID, spotLightPosID;
GLuint camtransID, camPosID;

DirectionalLight* directionalLight1;
PointLight *pointLight1, *pointLight2;
SpotLight* spotLight1;
static Camera* pGameCamera =
    new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, 45.0f, 0.1f, 1000.0f);
float Scale;
// GLfloat light[]= {0.0,1.0,-1.0,1.0};
int spfaces;
int spverts;
Line* xline;
Line* yline;
Line* zline;
Line* dirLightLine;
Mesh TestMesh, Plane, Cube;
SkyBox* skybox1;
TextLine2d *tline1, *tline2;
FontLine2d* fLine1;
PerlinNoise* noise1;
Billboard* bb1;
ShadowMapFBO* smfbo1;

// сигнал о том, что прогрузка шейдеров и пр. завершилась
bool initialized;

std::shared_ptr<Material> mainMaterial, secondMaterial, shadowMaterial;
std::shared_ptr<Texture2D> colorMap1, colorMap2, whiteTexture;
float fps;
int frameCount;
double lastTime;
enum class RenderType : uint8_t {
    Deferred = 0,
    Position,
    Diffuse,
    Normal,
    TextureCoordinates,
    Forward
};

RenderType renderType{RenderType::Deferred};

void CalcFPS() {
    double currentTime = glfwGetTime();
    ++frameCount;
    if (currentTime - lastTime >= 1.0) {
        fps = static_cast<double>(frameCount) / (currentTime - lastTime);
        lastTime += 1.0;
        frameCount = 0;
    }
}

void ErrorCallback(int error, const char* description) {
    std::cerr << "Error " << error << ":" << description;
}
void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_I) spotLight1->target[0] += 0.1;
        if (key == GLFW_KEY_K) spotLight1->target[0] -= 0.1;
        if (key == GLFW_KEY_O) spotLight1->target[1] += 0.1;
        if (key == GLFW_KEY_L) spotLight1->target[1] -= 0.1;
        if (key == GLFW_KEY_U) spotLight1->target[2] += 0.1;
        if (key == GLFW_KEY_J) spotLight1->target[2] -= 0.1;
        if (key == GLFW_KEY_F5) {
            renderType =
                static_cast<RenderType>(static_cast<uint8_t>(renderType) + 1);
            renderType =
                static_cast<RenderType>(static_cast<uint8_t>(renderType) % 6);
        }
        if (key == GLFW_KEY_PRINT_SCREEN) {
            time_t rawtime;
            struct tm* timeinfo;
            char buffer[80];

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer, 80, "screenshots/Screenshot %d-%m-%Y %I.%M.%S.png",
                     timeinfo);
            int result = SOIL_save_screenshot(buffer, SOIL_SAVE_TYPE_PNG, 0, 0,
                                              width, height);
            if (result != 0) {
                std::cout << "\n Screenshot saved as " << buffer;
            }
        } else {
            pGameCamera->OnKeyboard(key);
        }
    }
}
void MousePosCallBack(GLFWwindow* window, double x, double y) {
    mouse.Update(x, y);
    if (mouse.rightButtonPressed || true)
        pGameCamera->OnMouse(mouse.posX, mouse.posY);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_2) {  // right button
        if (action == GLFW_PRESS) {
            mouse.rightButtonPressed = true;
        } else {
            mouse.rightButtonPressed = false;
        }
    }
}

void FrameBufferSizeCallback(GLFWwindow* window, int w, int h) {
    width = w;
    height = h;
    if (tline1 != NULL) tline1->SetAspectRatio(width, height);
    if (fLine1 != NULL) fLine1->SetAspectRatio(width, height);
    if (smfbo1 != NULL) smfbo1->Init(w, h);
    if (gBuffer1 != NULL) gBuffer1->Init(w, h);
    pGameCamera->OnViewportResize(width, height);
    glViewport(0, 0, width, height);
}

void ShadowPass() {
    smfbo1->BindForWriting();
    // gBuffer1->BindForWriting();
    // этап рисовки
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // CalcFPS();
    Scale += 0.021f;
    TestMesh.SetRotation(0, 30 * sinf(Scale), 0);
    TestMesh.SetScale(0.02, 0.02, 0.02);
    TestMesh.SetPosition(0, -0.2, 0);
    // light3->SetPos(pGameCamera->GetPos());
    // light3->SetDir(pGameCamera->GetTarget()-pGameCamera->GetPos());
    Camera* lightCam =
        new Camera(width, height, 45, 1, 1000.0f, spotLight1->GetPos(),
                   glm::vec3(-1.0, -1.0, -1.0), glm::vec3(0.0, 1.0, 0.0));

    Plane.SetScale(30.0f, 30.0f, 30.0f);
    Plane.SetPosition(0.0f, -3.0f, 0.0f);
    Plane.SetRotation(0.0, 0.0, 0.0);

    glm::mat4 projection = glm::perspectiveFov(
        lightCam->GetFov(), static_cast<float>(lightCam->GetWidth()),
        static_cast<float>(lightCam->GetHeight()), lightCam->GetZNear(),
        lightCam->GetZFar());
    glm::mat4 view = glm::lookAt(lightCam->GetPos(), lightCam->GetTarget(),
                                 lightCam->GetUp());
    glm::mat4 vp_matrix = projection * view;

    shadowShader->Use();
    gCamViewID = shadowShader->GetUniformLocation("view_projection");
    rotation_id = shadowShader->GetUniformLocation("model_rotation");
    camPosID = shadowShader->GetUniformLocation("s_vCamPos");

    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, glm::value_ptr(vp_matrix));
    // освещение
    {
        // направленный свет
        auto la = glm::diagonal4x4(directionalLight1->color);
        glUniformMatrix4fv(dirLightColID, 1, GL_TRUE, glm::value_ptr(la));
        glUniform3f(dirLightDirID, directionalLight1->direction[0],
                    directionalLight1->direction[1],
                    directionalLight1->direction[2]);
        // точечный
        auto la2 = glm::diagonal4x4(pointLight1->color);
        glUniformMatrix4fv(pointLightColID, 1, GL_TRUE, glm::value_ptr(la2));
        glUniform3f(pointLightPosID, pointLight1->position[0],
                    pointLight1->position[1], pointLight1->position[2]);
        glUniform1f(pointLightIntID, pointLight1->power);
        // прожектор
        auto la3 = glm::diagonal4x4(spotLight1->color);
        glUniformMatrix4fv(spotLightColID, 1, GL_TRUE, glm::value_ptr(la3));
        glUniform3f(spotLightDirID, spotLight1->direction[0],
                    spotLight1->direction[1], spotLight1->direction[2]);
        glUniform1f(spotLightCutoffID,
                    cosf((spotLight1->Cutoff) * 3.14 / 180.0f));
        glUniform3f(spotLightPosID, spotLight1->position[0],
                    spotLight1->position[1], spotLight1->position[2]);
    }

    // вращение камеры для спекуляра
    glUniform3f(camPosID, lightCam->GetPos().x, lightCam->GetPos().y,
                lightCam->GetPos().z);

    Cube.SetMaterial(shadowMaterial);
    for (float i = -5.0f; i < 5.0f; i += 0.1f)
        for (float j = -5.0f; j < 5.0f; j += 0.1f) {
            Cube.SetScale(0.05f, 0.05f, 0.05f);
            Cube.SetRotation(0, 30 * sinf(Scale), 0);
            Cube.SetPosition(i, noise1->GetHeight(i, j), j);
            Cube.Render(lightCam);
        }
    Plane.SetMaterial(shadowMaterial);
    Plane.Render(lightCam);

    delete lightCam;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass() {
    // этап рисовки
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // CalcFPS();
    Scale += 0.021f;
    TestMesh.SetRotation(0, 30 * sinf(Scale), 0);
    TestMesh.SetScale(0.02, 0.02, 0.02);
    TestMesh.SetPosition(0, -0.2, 0);

    Plane.SetScale(30.0f, 30.0f, 30.0f);
    Plane.SetPosition(0.0f, -3.0f, 0.0f);
    Plane.SetRotation(0.0, 0.0, 0.0);
    glm::mat4 projection1 = glm::perspectiveFov(
        pGameCamera->GetFov(), static_cast<float>(pGameCamera->GetWidth()),
        static_cast<float>(pGameCamera->GetHeight()), pGameCamera->GetZNear(),
        pGameCamera->GetZFar());
    glm::mat4 view1 = glm::lookAt(
        pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    glm::mat4 vp_matrix1 = projection1 * view1;

    Camera* lightCam = new Camera(
        width, height, pGameCamera->GetFov(), pGameCamera->GetZNear(),
        pGameCamera->GetZFar(), spotLight1->GetPos(),
        glm::vec3(-1.0, -1.0, -1.0), glm::vec3(0.0, 1.0, 0.0));

    glm::mat4 projection2 = glm::perspectiveFov(
        lightCam->GetFov(), static_cast<float>(lightCam->GetWidth()),
        static_cast<float>(lightCam->GetHeight()), lightCam->GetZNear(),
        lightCam->GetZFar());
    glm::mat4 view2 = glm::lookAt(lightCam->GetPos(), lightCam->GetTarget(),
                                  lightCam->GetUp());
    glm::mat4 vp_matrix2 = projection2 * view2;

    skybox1->Render(pGameCamera);

    Plane.SetMaterial(mainMaterial);
    // тупо копипаст
    {
        meshShader->Use();
        GLuint gLightCamViewID = meshShader->GetUniformLocation("gLightVC");
        gCamViewID = meshShader->GetUniformLocation("view_projection");
        rotation_id = meshShader->GetUniformLocation("model_rotation");
        camPosID = meshShader->GetUniformLocation("s_vCamPos");

        glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, glm::value_ptr(vp_matrix1));
        glUniformMatrix4fv(gLightCamViewID, 1, GL_TRUE,
                           glm::value_ptr(vp_matrix2));

        // освещение
        {
            // направленный свет
            auto la = glm::diagonal4x4(directionalLight1->color);
            glUniformMatrix4fv(dirLightColID, 1, GL_TRUE, glm::value_ptr(la));
            glUniform3f(dirLightDirID, directionalLight1->direction[0],
                        directionalLight1->direction[1],
                        directionalLight1->direction[2]);
            // точечный
            auto la2 = glm::diagonal4x4(pointLight1->color);
            glUniformMatrix4fv(pointLightColID, 1, GL_TRUE,
                               glm::value_ptr(la2));
            glUniform3f(pointLightPosID, pointLight1->position[0],
                        pointLight1->position[1], pointLight1->position[2]);
            glUniform1f(pointLightIntID, pointLight1->power);

            // прожектор
            auto la3 = glm::diagonal4x4(spotLight1->color);
            glUniformMatrix4fv(spotLightColID, 1, GL_TRUE, glm::value_ptr(la3));
            glUniform3f(spotLightDirID, spotLight1->direction[0],
                        spotLight1->direction[1], spotLight1->direction[2]);
            glUniform1f(spotLightCutoffID,
                        cosf((spotLight1->Cutoff) * 3.14 / 180.0f));
            glUniform3f(spotLightPosID, spotLight1->position[0],
                        spotLight1->position[1], spotLight1->position[2]);
        }

        // вращение камеры для спекуляра
        glUniform3f(camPosID, pGameCamera->GetPos().x, pGameCamera->GetPos().y,
                    pGameCamera->GetPos().z);
    }
    // Plane.SetMaterial(mainMaterial);
    Plane.Render(pGameCamera);
    Cube.SetMaterial(mainMaterial);
    for (float i = -5.0f; i < 5.0f; i += 0.1f)
        for (float j = -5.0f; j < 5.0f; j += 0.1f) {
            Cube.SetScale(0.05f, 0.05f, 0.05f);
            Cube.SetRotation(0, 30 * sinf(Scale), 0);
            Cube.SetPosition(i, noise1->GetHeight(i, j), j);
            Cube.Render(pGameCamera);
        }
    // delete tempTexture;

    bb1->Render(pGameCamera);

    delete lightCam;
}

void DSBeginLightPasses() {
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // gBuffer1->BindForReading();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DSEndLigtPasses() { glDisable(GL_BLEND); }

void DSLightingPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gBuffer1->BindForReading();

    switch (renderType) {
        case RenderType::Position:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_POSITION);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case RenderType::Diffuse:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_DIFFUSE);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case RenderType::Normal:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        case RenderType::TextureCoordinates:
            gBuffer1->SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_TEXCOORD);
            glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            break;
        default:
            break;
    }
}

void DSStencilPass(Light& light) {
    // m_nullTech.Enable();
    // включаем шейдер
    DSStencilPassShader->Use();

    // Отключаем запись цвета / глубины и включаем трафарет
    gBuffer1->BindForStencilPass();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  //??

    glDisable(GL_CULL_FACE);

    glClear(GL_STENCIL_BUFFER_BIT);

    // Нам нужен тест трафарета, но мы хотим, что бы он всегда
    // успешно проходил. Важен только тест глубины.
    glStencilFunc(GL_ALWAYS, 0, 0);

    // glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR, GL_KEEP);
    // glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR, GL_KEEP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR,
                        GL_KEEP);  // TODO сделать наоборот
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR,
                        GL_KEEP);  // TODO сделать наоборот

    glm::mat4 projection1 = glm::perspectiveFov(
        pGameCamera->GetFov(), static_cast<float>(pGameCamera->GetWidth()),
        static_cast<float>(pGameCamera->GetHeight()), pGameCamera->GetZNear(),
        pGameCamera->GetZFar());
    glm::mat4 view1 = glm::lookAt(
        pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    glm::mat4 vp_matrix1 = projection1 * view1;

    // определяем адрес переменных камеры
    gCamViewID = DSStencilPassShader->GetUniformLocation("view_projection");
    rotation_id = DSStencilPassShader->GetUniformLocation("model_rotation");
    camPosID = DSStencilPassShader->GetUniformLocation("s_vCamPos");

    // загружаем матрицу камеры
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, glm::value_ptr(vp_matrix1));

    light.SetMaterial(DSStencilPassMaterial);
    light.Render(pGameCamera);
}

void DSPointLightPass(PointLight& pointLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glm::mat4 projection1 = glm::perspectiveFov(
        pGameCamera->GetFov(), static_cast<float>(pGameCamera->GetWidth()),
        static_cast<float>(pGameCamera->GetHeight()), pGameCamera->GetZNear(),
        pGameCamera->GetZFar());
    glm::mat4 view1 = glm::lookAt(
        pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    glm::mat4 vp_matrix1 = projection1 * view1;

    // включаем шейдер
    DSPointLightShader->Use();

    // определяем адрес переменных камеры
    gCamViewID = DSPointLightShader->GetUniformLocation("view_projection");
    rotation_id = DSPointLightShader->GetUniformLocation("model_rotation");
    camPosID = DSPointLightShader->GetUniformLocation("s_vCamPos");

    // загружаем матрицу камеры
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, glm::value_ptr(vp_matrix1));
    // взагружаем вращение камеры для спекуляра
    glUniform3f(camPosID, pGameCamera->GetPos().x, pGameCamera->GetPos().y,
                pGameCamera->GetPos().z);

    // загружаем текстуры в шейдер
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(0), 4);  // world pos
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(1), 5);  // diffuse
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(2), 6);  // normal
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(3), 7);  // UV
    DSPointLightMaterial->SetTexture(gBuffer1->GetTexture(4), 8);  // specular

    // включаем данные из буффера
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClear(/*GL_COLOR_BUFFER_BIT |*/GL_DEPTH_BUFFER_BIT);

    // определяем адрес параметров света
    pointLightPosID =
        DSPointLightShader->GetUniformLocation("s_vPointLightPos");
    pointLightIntID =
        DSPointLightShader->GetUniformLocation("pointLightIntensity");
    pointLightColID = DSPointLightShader->GetUniformLocation("pointLightColor");

    // загружаем параметры света для источника 1
    auto la2 = glm::diagonal4x4(pointLight.color);
    glUniformMatrix4fv(pointLightColID, 1, GL_TRUE, glm::value_ptr(la2));
    glUniform3f(pointLightPosID, pointLight.position[0], pointLight.position[1],
                pointLight.position[2]);
    glUniform1f(pointLightIntID, pointLight.power);

    pointLight.SetMaterial(DSPointLightMaterial);
    pointLight.Render(pGameCamera);

    glCullFace(GL_FRONT);
    glDisable(GL_BLEND);
}

void DSSpotLightPass(SpotLight& spotLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glm::mat4 projection1 = glm::perspectiveFov(
        pGameCamera->GetFov(), static_cast<float>(pGameCamera->GetWidth()),
        static_cast<float>(pGameCamera->GetHeight()), pGameCamera->GetZNear(),
        pGameCamera->GetZFar());
    glm::mat4 view1 = glm::lookAt(
        pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    glm::mat4 vp_matrix1 = projection1 * view1;

    // включаем шейдер
    DSSpotLightShader->Use();

    // определяем адрес переменных камеры
    gCamViewID = DSSpotLightShader->GetUniformLocation("view_projection");
    rotation_id = DSSpotLightShader->GetUniformLocation("model_rotation");
    camPosID = DSSpotLightShader->GetUniformLocation("s_vCamPos");

    // загружаем матрицу камеры
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, glm::value_ptr(vp_matrix1));
    // взагружаем вращение камеры для спекуляра
    glUniform3f(camPosID, pGameCamera->GetPos().x, pGameCamera->GetPos().y,
                pGameCamera->GetPos().z);

    // загружаем текстуры в шейдер
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(0), 4);  // world pos
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(1), 5);  // diffuse
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(2), 6);  // normal
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(3), 7);  // UV
    DSSpotLightMaterial->SetTexture(gBuffer1->GetTexture(4), 8);  // specular

    // определяем адрес параметров света
    spotLightPosID = DSSpotLightShader->GetUniformLocation("sLightPos");
    spotLightColID = DSSpotLightShader->GetUniformLocation("sLightCol");
    spotLightDirID = DSSpotLightShader->GetUniformLocation("sLightDir");
    spotLightCutoffID = DSSpotLightShader->GetUniformLocation("sLightCutoff");

    // загружаем параметры света для источника 1
    auto la2 = glm::diagonal4x4(spotLight.color);
    glUniform3f(spotLightPosID, spotLight.position[0], spotLight.position[1],
                spotLight.position[2]);
    glUniformMatrix4fv(spotLightColID, 1, GL_TRUE, glm::value_ptr(la2));
    glUniform3f(spotLightDirID, spotLight.direction[0], spotLight.direction[1],
                spotLight.direction[2]);
    glUniform1f(spotLightCutoffID, cosf(glm::radians(spotLight.Cutoff)));

    // включаем данные из буффера
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    spotLight.SetMaterial(DSSpotLightMaterial);
    spotLight.Render(pGameCamera);

    glCullFace(GL_FRONT);
    glDisable(GL_BLEND);
}

void DSDirectionalLightPass(DirectionalLight& directionalLight) {
    // gBuffer1->BindForReading();
    gBuffer1->BindForLightPass();
    // glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glm::mat4 projection1 = glm::perspectiveFov(
        pGameCamera->GetFov(), static_cast<float>(pGameCamera->GetWidth()),
        static_cast<float>(pGameCamera->GetHeight()), pGameCamera->GetZNear(),
        pGameCamera->GetZFar());
    glm::mat4 view1 = glm::lookAt(
        pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    glm::mat4 vp_matrix1 = projection1 * view1;

    DSDirectionalLightShader->Use();
    // получаем адрес параметров камеры
    gCamViewID =
        DSDirectionalLightShader->GetUniformLocation("view_projection");
    rotation_id =
        DSDirectionalLightShader->GetUniformLocation("model_rotation");
    camPosID = DSDirectionalLightShader->GetUniformLocation("s_vCamPos");
    // загружаем вращение камеры для спекуляра
    glUniform3f(camPosID, pGameCamera->GetPos().x, pGameCamera->GetPos().y,
                pGameCamera->GetPos().z);
    // загружаем матрицу камеры
    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, glm::value_ptr(vp_matrix1));

    // получаем адрес переменных света
    dirLightColID =
        DSDirectionalLightShader->GetUniformLocation("dirLightColor");
    dirLightDirID =
        DSDirectionalLightShader->GetUniformLocation("dirLightDirection");

    // загрузка параметров света
    auto la2 = glm::diagonal4x4(directionalLight.color);
    glUniformMatrix4fv(dirLightColID, 1, GL_TRUE, glm::value_ptr(la2));
    glUniform3f(dirLightDirID, directionalLight.direction[0],
                directionalLight.direction[1], directionalLight.direction[2]);

    // загружаем текстуры в шейдер
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

    // включаем данные из буффера
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);

    directionalLight.Render(pGameCamera);
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void DSFinalPass() {
    gBuffer1->BindForFinalPass();
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH,
                      WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void InterfacePass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glDisable(GL_DEPTH_TEST);
    xline->Render(pGameCamera);
    yline->Render(pGameCamera);
    zline->Render(pGameCamera);
    std::string strCampos = std::to_string(pGameCamera->GetPos().x) + "; " +
                            std::to_string(pGameCamera->GetPos().y) + "; " +
                            std::to_string(pGameCamera->GetPos().z);
    // fLine1->Render((strCampos).c_str(),-1.0f,0.0f,24.0f);
    fLine1->SetText((strCampos).c_str());
    fLine1->SetPosition(-1.0f, 0.0f, 24.0f);
    fLine1->Render(pGameCamera);
    // gBuffer1->CheckTextures();
    CalcFPS();
    // fLine1->Render(ConvertToString(fps),-1.0f,0.9f,24.0f);
    fLine1->SetText(std::to_string(fps));
    fLine1->SetPosition(-1.0f, 0.9f, 24.0f);
    fLine1->Render(pGameCamera);

    switch (renderType) {
        case RenderType::Deferred:
            fLine1->SetText("Deferred shading");
            break;
        case RenderType::Position:
            fLine1->SetText("Position printing");
            break;
        case RenderType::Diffuse:
            fLine1->SetText("Diffuse printing");
            break;
        case RenderType::Normal:
            fLine1->SetText("Normal printing");
            break;
        case RenderType::TextureCoordinates:
            fLine1->SetText("UV printing");
            break;
        case RenderType::Forward:
            fLine1->SetText("Forward rendering");
            break;
        default:
            fLine1->SetText("Unknown");
            break;
    }
    fLine1->SetPosition(-1.0f, -0.2f, 36.0f);
    fLine1->Render(pGameCamera);

    dirLightLine->Render(pGameCamera);
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void DSGeometryPass() {
    gBuffer1->BindForGeomPass();
    // gBuffer1->BindForWriting();
    // Только геометрический проход обновляет тест глубины
    glDepthMask(GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    CalcFPS();

    glm::mat4 projection1 = glm::perspectiveFov(
        pGameCamera->GetFov(), static_cast<float>(pGameCamera->GetWidth()),
        static_cast<float>(pGameCamera->GetHeight()), pGameCamera->GetZNear(),
        pGameCamera->GetZFar());
    glm::mat4 view1 = glm::lookAt(
        pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    glm::mat4 vp_matrix1 = projection1 * view1;

    DSGeometryPassShader->Use();
    gCamViewID = DSGeometryPassShader->GetUniformLocation("view_projection");

    glUniformMatrix4fv(gCamViewID, 1, GL_TRUE, glm::value_ptr(vp_matrix1));

    Cube.SetMaterial(DSGeometryPassMaterial);
    for (float i = -5.0f; i < 5.0f; i += 0.1f)
        for (float j = -5.0f; j < 5.0f; j += 0.1f) {
            Cube.SetScale(0.05f, 0.05f, 0.05f);
            Cube.SetRotation(0, 30 * sinf(Scale), 0);
            Cube.SetPosition(i, noise1->GetHeight(i, j), j);
            Cube.Render(pGameCamera);
        }

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}

void RenderScene(GLFWwindow* window) {
    // ShadowPass();
    // RenderPass();
    // DSGeometryPass();
    // DSBeginLightPasses();//вообще пока не нужно
    // DSPointLightPass();
    // DSLightingPass();
    // обычный deffered shading
    switch (renderType) {
        case RenderType::Deferred:
            gBuffer1->StartFrame();
            DSGeometryPass();
            // Для того, что бы обновился буфер трафарета нужно его
            // активировать, так же он потребуется и в проходе света, так как
            // свет рендерится только при успешном проходе трафарета.
            glEnable(GL_STENCIL_TEST);
            // DSBeginLightPasses();//уже устарело
            {
                DSStencilPass(*pointLight1);
                DSPointLightPass(*pointLight1);

                DSStencilPass(*pointLight2);
                DSPointLightPass(*pointLight2);

                DSStencilPass(*spotLight1);
                DSSpotLightPass(*spotLight1);
            }
            // Направленному свету не требуется трафарет
            // так как его действие не ограничено расстоянием.
            glDisable(GL_STENCIL_TEST);

            DSDirectionalLightPass(*directionalLight1);

            DSFinalPass();
            // DSEndLigtPasses();
            break;
        case RenderType::Position:
            [[fallthrough]];
        case RenderType::Diffuse:
            [[fallthrough]];
        case RenderType::Normal:
            [[fallthrough]];
        case RenderType::TextureCoordinates:
            DSGeometryPass();
            DSLightingPass();
            break;
        case RenderType::Forward:
            RenderPass();
            break;
        default:
            break;
    }
    InterfacePass();
}
void PreInitScene(GLFWwindow* window) {
    Scale = 0;
    lastTime = glfwGetTime();
    frameCount = 0;

    // шейдер текста
    {
        std::ifstream vertex_shader_file("shaders/text2d.vs");
        std::string vertex_shader_text(
            (std::istreambuf_iterator<char>(vertex_shader_file)),
            (std::istreambuf_iterator<char>()));

        std::ifstream fragment_shader_file("shaders/text2d.fs");
        std::string fragment_shader_text(
            (std::istreambuf_iterator<char>(fragment_shader_file)),
            (std::istreambuf_iterator<char>()));

        textShader = std::make_shared<Shader>();
        textShader->AddShader(vertex_shader_text, VertexShader);
        textShader->AddShader(fragment_shader_text, FragmnetShader);
        textShader->Init();
    }
    fLine1 = new FontLine2d();
    fLine1->Init(std::string("fonts/MagistralIC_UTF-8.fnt"), textShader);
    fLine1->SetAspectRatio(width, height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // fLine1->Render("Loading...",-1.0f,-0.1f,72.0f);
    fLine1->SetText("Loading...");
    fLine1->SetPosition(-1.0f, -0.1f, 72.0f);
    fLine1->Render(pGameCamera);
    glfwSwapBuffers(window);
    initialized = false;
}

void InitRender(GLFWwindow* window, std::string message) {
    // thread thr(InitScene,hiddenWindow);

    // thr.detach();
    // InitScene(window);

    // while(!glfwWindowShouldClose(window) && initialized == false)
    if (initialized == false) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CalcFPS();
        // fLine1->Render(ConvertToString(fps),-1.0f,0.9f,24.0f);
        fLine1->SetText(std::to_string(fps));
        fLine1->SetPosition(-1.0f, 0.9f, 24.0f);
        fLine1->Render(pGameCamera);

        // fLine1->Render(message,-1.0f,-0.1f,36.0f);
        fLine1->SetText(message);
        fLine1->SetPosition(-1.0f, -0.1f, 36.0f);
        fLine1->Render(pGameCamera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // thr.join();
}

int InitScene(GLFWwindow* window) {
    /*std::thread thr(InitRender, window, string("test"));

    thr.detach();*/

    initialized = false;
    glfwMakeContextCurrent(window);
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    } else {
        printf("\nGLEW status is %d \n", res);
    }
    /*Scale=0;
    lastTime = glfwGetTime();
    frameCount=0;

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);**/

    // нормальный шейдер
    // InitRender(window, "Normal shader loading...");

    {
        std::ifstream vertex_shader_file("shaders/vertexShader.vs");
        std::string vertex_shader_text(
            (std::istreambuf_iterator<char>(vertex_shader_file)),
            (std::istreambuf_iterator<char>()));

        std::ifstream fragment_shader_file("shaders/fragmentShader.fs");
        std::string fragment_shader_text(
            (std::istreambuf_iterator<char>(fragment_shader_file)),
            (std::istreambuf_iterator<char>()));

        meshShader = std::make_shared<Shader>();
        meshShader->AddShader(vertex_shader_text, VertexShader);
        meshShader->AddShader(fragment_shader_text, FragmnetShader);
        meshShader->Init();

        gCamViewID = meshShader->GetUniformLocation("view_projection");
        rotation_id = meshShader->GetUniformLocation("model_rotation");
        camPosID = meshShader->GetUniformLocation("s_vCamPos");
    }

    // основной материал
    {
        InitRender(window, "Main material loading...");
        mainMaterial = std::make_shared<Material>();
        mainMaterial->Init(meshShader);
    }

    // Cube.Init(mainMaterial,"models/sphere2.ho3d");
    TestMesh.Init(mainMaterial, "models/torus1.ho3d");
    // Plane.Init(mainMaterial, "models/plane.ho3d");

    // шейдер тени
    {
        std::ifstream vertex_shader_file("shaders/fbo.vs");
        std::string vertex_shader_text(
            (std::istreambuf_iterator<char>(vertex_shader_file)),
            (std::istreambuf_iterator<char>()));

        std::ifstream fragment_shader_file("shaders/fbo.fs");
        std::string fragment_shader_text(
            (std::istreambuf_iterator<char>(fragment_shader_file)),
            (std::istreambuf_iterator<char>()));

        InitRender(window, "Shade shader loading...");
        shadowShader = std::make_shared<Shader>();
        shadowShader->AddShader(vertex_shader_text, VertexShader);
        shadowShader->AddShader(fragment_shader_text, FragmnetShader);
        shadowShader->Init();
    }

    // материал тени
    {
        InitRender(window, "Shade Material loading...");
        shadowMaterial = std::make_shared<Material>();
        shadowMaterial->Init(shadowShader);
    }

    // затенённый шейдер
    {
        InitRender(window, "Shaded shader loading...");

        std::ifstream vertex_shader_file("shaders/shadowed.vs");
        std::string vertex_shader_text(
            (std::istreambuf_iterator<char>(vertex_shader_file)),
            (std::istreambuf_iterator<char>()));

        std::ifstream fragment_shader_file("shaders/shadowed.fs");
        std::string fragment_shader_text(
            (std::istreambuf_iterator<char>(fragment_shader_file)),
            (std::istreambuf_iterator<char>()));
        shadowMeshShader = std::make_shared<Shader>();
        shadowMeshShader->AddShader(vertex_shader_text, VertexShader);
        shadowMeshShader->AddShader(fragment_shader_text, FragmnetShader);
        shadowMeshShader->Init();
    }

    // затенённый материал
    {
        InitRender(window, "Shaded material loading...");
        shadowMeshMaterial = std::make_shared<Material>();
        shadowMeshMaterial->Init(shadowMeshShader);
        whiteTexture = std::make_shared<Texture2D>();
        whiteTexture->Load("Textures/white.png");
        shadowMeshMaterial->SetColorTexture(whiteTexture);
        Cube.Init(shadowMeshMaterial, "models/normal_geosphere.ho3d");
        Plane.Init(shadowMeshMaterial, "models/normal_plane.ho3d");
    }

    // шейдер StencilPass материал
    {
        InitRender(window, "StencilPass shader loading...");
        // грузим шейдер
        {
            std::ifstream vertex_shader_file("shaders/DSStencilPass.vs");
            std::string vertex_shader_text(
                (std::istreambuf_iterator<char>(vertex_shader_file)),
                (std::istreambuf_iterator<char>()));
            DSStencilPassShader = std::make_shared<Shader>();
            DSStencilPassShader->AddShader(vertex_shader_text, VertexShader);
            DSStencilPassShader->Init();
        }
        // грузим материал
        {
            DSStencilPassMaterial = std::make_shared<Material>();
            DSStencilPassMaterial->Init(DSStencilPassShader);
        }
    }

    // шейдер GBufferа и сам буффер, материал
    {
        InitRender(window, "Gbuffer shader loading...");
        gBuffer1 = new GBuffer;
        gBuffer1->Init(width, height);
        // грузим шейдер
        {
            std::ifstream vertex_shader_file("shaders/DSGeometryPass.vs");
            std::string vertex_shader_text(
                (std::istreambuf_iterator<char>(vertex_shader_file)),
                (std::istreambuf_iterator<char>()));

            std::ifstream fragment_shader_file("shaders/DSGeometryPass.fs");
            std::string fragment_shader_text(
                (std::istreambuf_iterator<char>(fragment_shader_file)),
                (std::istreambuf_iterator<char>()));

            DSGeometryPassShader = std::make_shared<Shader>();
            DSGeometryPassShader->AddShader(vertex_shader_text, VertexShader);
            DSGeometryPassShader->AddShader(fragment_shader_text,
                                            FragmnetShader);
            DSGeometryPassShader->Init();
        }
        // грузим материал
        {
            DSGeometryPassMaterial = std::make_shared<Material>();
            DSGeometryPassMaterial->Init(DSGeometryPassShader);
        }
    }

    // шейдера света
    {
        InitRender(window, "Light shader loading...");
        // шейдер точечного света
        // грузим шейдер
        {
            std::ifstream vertex_shader_file("shaders/DSPointLight.vs");
            std::string vertex_shader_text(
                (std::istreambuf_iterator<char>(vertex_shader_file)),
                (std::istreambuf_iterator<char>()));

            std::ifstream fragment_shader_file("shaders/DSPointLight.fs");
            std::string fragment_shader_text(
                (std::istreambuf_iterator<char>(fragment_shader_file)),
                (std::istreambuf_iterator<char>()));

            DSPointLightShader = std::make_shared<Shader>();
            DSPointLightShader->AddShader(vertex_shader_text, VertexShader);
            DSPointLightShader->AddShader(fragment_shader_text, FragmnetShader);
            DSPointLightShader->Init();

            // грузим материал
            DSPointLightMaterial = std::make_shared<Material>();
            DSPointLightMaterial->Init(DSPointLightShader);
        }
        // шейдер направленного света
        {
            // грузим шейдер
            std::ifstream vertex_shader_file("shaders/DSDirectionalLight.vs");
            std::string vertex_shader_text(
                (std::istreambuf_iterator<char>(vertex_shader_file)),
                (std::istreambuf_iterator<char>()));

            std::ifstream fragment_shader_file("shaders/DSDirectionalLight.fs");
            std::string fragment_shader_text(
                (std::istreambuf_iterator<char>(fragment_shader_file)),
                (std::istreambuf_iterator<char>()));
            DSDirectionalLightShader = std::make_shared<Shader>();
            DSDirectionalLightShader->AddShader(vertex_shader_text,
                                                VertexShader);
            DSDirectionalLightShader->AddShader(fragment_shader_text,
                                                FragmnetShader);
            DSDirectionalLightShader->Init();

            // грузим материал

            DSDirectionalLightMaterial = std::make_shared<Material>();
            DSDirectionalLightMaterial->Init(DSDirectionalLightShader);
        }
        // шейдер прожектора света
        {
            // грузим шейдер
            std::ifstream vertex_shader_file("shaders/DSSpotLight.vs");
            std::string vertex_shader_text(
                (std::istreambuf_iterator<char>(vertex_shader_file)),
                (std::istreambuf_iterator<char>()));

            std::ifstream fragment_shader_file("shaders/DSSpotLight.fs");
            std::string fragment_shader_text(
                (std::istreambuf_iterator<char>(fragment_shader_file)),
                (std::istreambuf_iterator<char>()));

            DSSpotLightShader = std::make_shared<Shader>();
            DSSpotLightShader->AddShader(vertex_shader_text, VertexShader);
            DSSpotLightShader->AddShader(fragment_shader_text, FragmnetShader);
            DSSpotLightShader->Init();

            // грузим материал
            DSSpotLightMaterial = std::make_shared<Material>();
            DSSpotLightMaterial->Init(DSSpotLightShader);
        }
    }

    // шейдер скайбокса
    {
        InitRender(window, "Skybox shader loading...");
        std::ifstream vertex_shader_file("shaders/skybox.vs");
        std::string vertex_shader_text(
            (std::istreambuf_iterator<char>(vertex_shader_file)),
            (std::istreambuf_iterator<char>()));

        std::ifstream fragment_shader_file("shaders/skybox.fs");
        std::string fragment_shader_text(
            (std::istreambuf_iterator<char>(fragment_shader_file)),
            (std::istreambuf_iterator<char>()));

        skyboxShader = std::make_shared<Shader>();

        skyboxShader->AddShader(vertex_shader_text, VertexShader);
        skyboxShader->AddShader(fragment_shader_text, FragmnetShader);
        skyboxShader->Init();
    }

    skybox1 = new SkyBox(skyboxShader);
    skybox1->Init("textures/canyon1.jpg");
    // настройка света и единичных векторов
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

        directionalLight1 =
            new DirectionalLight(-1.5f, -1.0f, -1.5f,  // direction
                                 0.5f, 0.5f, 0.5f,     // color
                                 DSDirectionalLightMaterial);
        pointLight1 = new PointLight(0, 1, -0.2,     // position
                                     1.0, 1.0, 1.0,  // color
                                     1.0, DSPointLightMaterial);
        pointLight2 = new PointLight(1.5, 0.4, 0.0,  // position
                                     0.3, 0.3, 1.0,  // color
                                     1.1,            // power
                                     DSPointLightMaterial);
        spotLight1 = new SpotLight(1.5f, 0.0f, 0.5f,  // target
                                   1.0f, 0.4f, 0.4f,  // color
                                   0.0f, 1.0f, 0.0f,  // position
                                   35.0f,             // cutoff in degrees
                                   DSSpotLightMaterial);
        glm::vec3 PX(1, 0, 0);
        glm::vec3 PY(0, 1, 0);
        glm::vec3 PZ(0, 0, 1);
        glm::vec3 P0(0, 0, 0);

        xline = new Line(PX, P0, PX);
        yline = new Line(PY, P0, PY, xline->GetShader());
        zline = new Line(PZ, P0, PZ, xline->GetShader());
        dirLightLine = new Line(P0, directionalLight1->GetDir(),
                                directionalLight1->GetCol());
    }

    // прочее
    {
        InitRender(window, "Final steps...");
        bb1 = new Billboard();
        bb1->Init("Textures/monster_hellknight.png");
        bb1->SetPos(glm::vec3(0, 0, 0));

        noise1 = new PerlinNoise(1, 10.3, 0.5, 2, 42);

        tline1 = new TextLine2d();
        tline2 = new TextLine2d();
        tline1->Init(width, height, textShader);
        tline2->Init(width, height, textShader);
        // fLine1->Init(string("fonts/MagistralIC_UTF-8.fnt"),textShader);
        // fLine1->SetAspectRatio(width,height);

        // *************Shadow MAP FBO**********/
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

int main(int argc, char** argv) {
    glfwWindowHint(GLFW_SAMPLES, 4);                // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);  // We don't want the old OpenGL
    GLFWwindow* window;
    glfwSetErrorCallback(reinterpret_cast<GLFWerrorfun>(&ErrorCallback));
    // glewExperimental = true; // Needed for core profile
    if (!glfwInit()) exit(EXIT_FAILURE);

    // заголовок
    std::string title("NEE");

    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(),
                              /*glfwGetPrimaryMonitor()*/ nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    hiddenWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(),
                                    nullptr, window);

    glfwSetFramebufferSizeCallback(
        window,
        reinterpret_cast<GLFWframebuffersizefun>(&FrameBufferSizeCallback));
    width = WINDOW_WIDTH;
    height = WINDOW_HEIGHT;
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, reinterpret_cast<GLFWkeyfun>(&KeyCallback));
    glfwSetCursorPosCallback(
        window, reinterpret_cast<GLFWcursorposfun>(&MousePosCallBack));
    glfwSetMouseButtonCallback(
        window, reinterpret_cast<GLFWmousebuttonfun>(&MouseButtonCallback));
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    } else {
        printf("\nGLEW status is %d \n", res);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // включаем прозрачность
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    PreInitScene(window);
    // InitRender(window,"test");
    if (InitScene(window) != 0) return -1;

    while (!glfwWindowShouldClose(window)) {
        RenderScene(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwDestroyWindow(hiddenWindow);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
