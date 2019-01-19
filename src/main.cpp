#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <GLFW/glfw3.h>

using namespace std::chrono_literals;

int main(int argc, char const *argv[])
{
    std::cout<<"Test";
    if (!glfwInit())
    {
        std::cout<<"Cannot init glfw"<<std::endl;
        return -1;
    }

    //auto window = std::make_unique<GLFWwindow>(glfwCreateWindow(640, 480, "My Title", NULL, NULL));
    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    if (!window)
    {
        // Window or OpenGL context creation failed
        std::cout<<"Cannot create Opengl context";
    }

    std::this_thread::sleep_for(10s);

    glfwTerminate();
    return 0;
}
