#ifndef APPLICATION_APPLICATION_H
#define APPLICATION_APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

#include "Camera/Camera.h"

class Application final {
public:
    Application();
    ~Application();
    int Run();

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
};

#endif /* APPLICATION_APPLICATION_H */
