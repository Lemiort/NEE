#include "Application.h"

#include "VulkanAdapter/VulkanAdapter.h"

Application::Application() : kCaption_{"NEE"} {
    LoadSettings();

    InitGlfw();

    switch (api_) {
        case GraphicsApi::kVulkan:
            InitVulkan();
            break;
        case GraphicsApi::kOpengl:
            InitOpengl();
            break;
        default:
            throw std::runtime_error("Unknow API");
            break;
    }
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
    if (j["GraphicsAPI"] == "OpenGL") {
        api_ = GraphicsApi::kOpengl;
    } else {
        api_ == GraphicsApi::kVulkan;
    }
}

void Application::InitGlfw() {
    glfwSetErrorCallback(ErrorCallback);

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Cannot init glfw");
    }
}

void Application::InitOpengl() {
    glfwWindowHint(GLFW_SAMPLES, 4);                // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);  // We don't want the old OpenGL

    window_ = std::unique_ptr<GLFWwindow, DestroyglfwWin>(
        glfwCreateWindow(width_, height_, kCaption_.c_str(), NULL, NULL));

    if (!window_) {
        throw std::runtime_error("Can not init glfw window");
    }

    glfwMakeContextCurrent(window_.get());

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        auto str = glewGetErrorString(res);
        throw std::runtime_error(reinterpret_cast<const char*>(str));
    }
}

void Application::InitVulkan() {
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