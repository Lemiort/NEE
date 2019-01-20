#ifndef APPLICATION_APPLICATION_H
#define APPLICATION_APPLICATION_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

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

    const std::string kCaption_;
    int width_;
    int height_;
    std::unique_ptr<GLFWwindow, DestroyglfwWin> window_;
};

#endif /* APPLICATION_APPLICATION_H */
