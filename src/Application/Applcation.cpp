#include "Application.h"

Application::Application() : kCaption_{"NEE"} {
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

    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    // error_callback_ = std::bind(&Application::ErrorCallback, this,
    //                             std::placeholders::_1,
    //                             std::placeholders::_2);
    // glfwSetErrorCallback(*error_callback_.target<GLFWerrorfun>());
    // if (!glfwInit()) {
    //     throw std::runtime_error("Can not init glfw");
    // }

    PFN_vkCreateInstance pfnCreateInstance =
        (PFN_vkCreateInstance)glfwGetInstanceProcAddress(NULL,
                                                         "vkCreateInstance");
    VkInstanceCreateInfo ici;
    memset(&ici, 0, sizeof(ici));
    ici.enabledExtensionCount = count;
    ici.ppEnabledExtensionNames = extensions;

    VkInstance vk_instance;

    auto resullt = pfnCreateInstance(&ici, nullptr, &vk_instance);
    if (resullt > 0) {
        throw std::runtime_error("Cannot create vk instance");
    }

    VkSurfaceKHR surface;
    VkResult err =
        glfwCreateWindowSurface(vk_instance, window_.get(), nullptr, &surface);
    if (err) {
        // Window surface creation failed
        throw std::runtime_error("Window surface creation failed");
    }
}

int Application::Run() {
    while (!glfwWindowShouldClose(window_.get())) {
        glfwSwapBuffers(window_.get());
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

Application::~Application() { glfwTerminate(); }

void Application::ErrorCallback(int error, const char* description) {
    throw std::runtime_error(description);
}