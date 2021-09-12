#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "gtest/gtest.h"

constexpr size_t kWindowWidth = 640;
constexpr size_t kWindowHeight = 480;
constexpr char kWindowTitle[] = "Test window";

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
        glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle,
                         /*glfwGetPrimaryMonitor()*/ nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cout << "glfwCreateWindow fails";
        return -1;
    }
    glfwMakeContextCurrent(window);

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cout << "GLEW error:" << glewGetErrorString(res) << std::endl;
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, kWindowWidth, kWindowHeight);

    auto result = RUN_ALL_TESTS();

    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}
