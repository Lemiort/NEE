#include "Application.h"
#include "VulkanAdapter/VulkanAdapter.h"

Application::Application() : kCaption_{"NEE"} {
    LoadSettings();

    InitGlfw();

    InitVulkan();
}

void Application::LoadSettings() {
    // read a JSON file
    std::ifstream ifstream("../config/settings.json");
    if (!ifstream)  // operator! is used here
    {
        throw std::runtime_error("Can not open setting file");
    }

    nlohmann::json j;
    ifstream >> j;

    width_ = j["window"]["width"];
    height_ = j["window"]["height"];
}

void Application::InitGlfw() {
    glfwSetErrorCallback(ErrorCallback);

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Cannot init glfw");
    }

    if (!glfwVulkanSupported()) {
        // Vulkan is available, at least for compute
        throw std::runtime_error("Vulkan is not supported");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_ = std::unique_ptr<GLFWwindow, DestroyglfwWin>(
        glfwCreateWindow(width_, height_, kCaption_.c_str(), NULL, NULL));

    if (!window_) {
        throw std::runtime_error("Can not init glfw window");
    }
}

void Application::InitVulkan() {
    VulkanAdapter vulkan;

    VkSurfaceKHR surface;
    VkResult err = glfwCreateWindowSurface(vulkan.GetVkInstance(),
                                           window_.get(), nullptr, &surface);
    if (err) {
        // Window surface creation failed
        throw std::runtime_error("Window surface creation failed");
    }
}

int Application::Run() {
    while (!glfwWindowShouldClose(window_.get())) {
        // glfwSwapBuffers(window_.get());
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

Application::~Application() {
    // SHUT UP
    glfwSetErrorCallback(nullptr);
    glfwTerminate();
}

void Application::ErrorCallback(int error, const char* description) {
    throw std::runtime_error(description);
}