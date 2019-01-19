#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace std::chrono_literals;

int main(int argc, char const *argv[])
{
    glfwWindowHint(GLFW_SAMPLES, 4);               // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
    GLFWwindow *window;
    // glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        std::cout << "Can not init glfw" << std::endl;
        exit(EXIT_FAILURE);
    }
    window = glfwCreateWindow(800, 600, "NEE", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallback);
    // width=WINDOW_WIDTH;
    // height=WINDOW_HEIGHT;
    glfwMakeContextCurrent(window);
    // glfwSetKeyCallback(window, KeyCallback);
    // glfwSetCursorPosCallback(window,MousePosCallBack);
    // glfwSetMouseButtonCallback(window, MouseButtonCallback);
    /*GLenum res = glewInit();
  if (res != GLEW_OK)
      {
          fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
          return 1;
      }
  else
  {
      printf("\nGLEW status is %d \n", res);
  }*/

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    // glEnable(GL_PROGRAM_POINT_SIZE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // //�������� ������������
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}