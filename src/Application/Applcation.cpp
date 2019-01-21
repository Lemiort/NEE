#include "Application.h"

Application::Application() : kCaption_{"NEE"} {
    glfwWindowHint(GLFW_SAMPLES, 4);                // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);  // We don't want the old OpenGL

    error_callback_ = std::bind(&Application::ErrorCallback, this,
                                std::placeholders::_1, std::placeholders::_2);
    glfwSetErrorCallback(*error_callback_.target<GLFWerrorfun>());
    if (!glfwInit()) {
        throw std::runtime_error("Can not init glfw");
    }

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

    window_ = std::unique_ptr<GLFWwindow, DestroyglfwWin>(
        glfwCreateWindow(width_, height_, kCaption_.c_str(), NULL, NULL));

    if (!window_) {
        throw std::runtime_error("Can not init glfw window");
    }
    // glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallback);
    glfwMakeContextCurrent(window_.get());
    // glfwSetKeyCallback(window, KeyCallback);
    // glfwSetCursorPosCallback(window,MousePosCallBack);
    // glfwSetMouseButtonCallback(window, MouseButtonCallback);
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        throw std::runtime_error(
            reinterpret_cast<const char*>(glewGetErrorString(res)));
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // //�������� ������������
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, width_, height_);
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