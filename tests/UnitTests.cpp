#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "gtest/gtest.h"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    auto errorCallback = [](int error, const char* description) -> void {
        FAIL() << description;
    };

    glfwWindowHint(GLFW_SAMPLES, 4);                // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);  // We don't want the old OpenGL
    glfwSetErrorCallback(reinterpret_cast<GLFWerrorfun>(&errorCallback));
    if (!glfwInit()) {
        std::cout << "glfwInit fails";
        return -1;
    }

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    auto window =
        glfwCreateWindow(640, 480, "test window",
                         /*glfwGetPrimaryMonitor()*/ nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cout << "glfwCreateWindow fails";
        return -1;
    }
    glfwMakeContextCurrent(window);

    return RUN_ALL_TESTS();
}
