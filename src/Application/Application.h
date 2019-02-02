#ifndef APPLICATION_APPLICATION_H
#define APPLICATION_APPLICATION_H

// #define GLFW_INCLUDE_VULKAN
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

#include "Font/Font.h"
#include "Mesh/WiredMesh.h"
#include "Shader/Shader.h"

class Application final {
public:
    Application();
    ~Application();
    int Run();

    enum class GraphicsApi { kOpengl, kVulkan };

private:
    struct DestroyglfwWin {
        void operator()(GLFWwindow* ptr) {
            if (ptr != nullptr) {
                glfwDestroyWindow(ptr);
            }
        }
    };

    static void ErrorCallback(int error, const char* description);

    const std::string kCaption_;
    int width_;
    int height_;
    std::unique_ptr<GLFWwindow, DestroyglfwWin> window_;
    GraphicsApi api_;
    std::vector<WiredMesh> meshes_;
    GLuint VBO, IBO;
    Shader shader_;
    Font font_;

    void LoadSettings();
    void InitGlfw();
    void InitVulkan();
    void InitOpengl();

    void OpenglRenderPass();
    void OpenglRenderLinesPass();
};

#endif /* APPLICATION_APPLICATION_H */
