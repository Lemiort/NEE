#include "Application.h"

// #include "VulkanAdapter/VulkanAdapter.h"

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

    WiredMesh orts;
    orts.vertexes = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    orts.lines = {{0, 1}, {0, 2}, {0, 3}};
    meshes_.push_back(orts);

    // glGenBuffers(1, &VBO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // glGenBuffers(1, &IBO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    std::string vertesx_shader =
        "#version 330 \n"
        "in vec3 position; \n"
        "void main() { \n"
        "gl_Position = /*gWVCP **/ vec4(position, 1.0);\n"
        "gl_PointSize=10;\n"
        "}\n";

    std::string fragment_shader =
        "#version 330 \n"
        "out vec4 FragColor;\n"
        "void main(){\n"
        "FragColor=vec4(1,0,0,1);\n"
        "}\n";

    shader_.Add(Shader::Type::kVertex, vertesx_shader);
    shader_.Add(Shader::Type::kFragmnet, fragment_shader);
    line_ = new Line();
}

Application::~Application() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);

    // SHUT UP
    glfwSetErrorCallback(nullptr);
    glfwTerminate();
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

    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, width_, height_);
}

void Application::InitVulkan() {
    // if (!glfwVulkanSupported()) {
    //     // Vulkan is available, at least for compute
    //     throw std::runtime_error("Vulkan is not supported");
    // }

    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    // window_ = std::unique_ptr<GLFWwindow, DestroyglfwWin>(
    //     glfwCreateWindow(width_, height_, kCaption_.c_str(), NULL, NULL));

    // if (!window_) {
    //     throw std::runtime_error("Can not init glfw window");
    // }

    // VulkanAdapter vulkan;

    // VkSurfaceKHR surface;
    // VkResult err = glfwCreateWindowSurface(vulkan.GetVkInstance(),
    //                                        window_.get(), nullptr, &surface);
    // if (err) {
    //     // Window surface creation failed
    //     throw std::runtime_error("Window surface creation failed");
    // }
}

int Application::Run() {
    while (!glfwWindowShouldClose(window_.get())) {
        switch (api_) {
            case GraphicsApi::kVulkan:
                // InitVulkan();
                break;
            case GraphicsApi::kOpengl:
                OpenglRenderPass();
                break;
            default:
                throw std::runtime_error("Unknow API");
                break;
        }
        glfwPollEvents();
    }
    return EXIT_SUCCESS;
}

void Application::ErrorCallback(int error, const char* description) {
    throw std::runtime_error(description);
}

void Application::OpenglRenderPass() {
    glClear(GL_COLOR_BUFFER_BIT);
    OpenglRenderLinesPass();
    glfwSwapBuffers(window_.get());
}

void Application::OpenglRenderLinesPass() {
    // for (auto& mesh : meshes_) {
    //     // shader_.Use();

    //     glEnableVertexAttribArray(0);

    //     // bind VBO
    //     glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //     // load data to VBO
    //     std::vector<float> vertexes;
    //     for (const auto& vertex : mesh.vertexes) {
    //         vertexes.push_back(vertex[0]);
    //         vertexes.push_back(vertex[1]);
    //         vertexes.push_back(vertex[2]);
    //     }
    //     glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexes.size(),
    //                  vertexes.data(), GL_STATIC_DRAW);

    //     // bind IBO
    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    //     // load indices
    //     std::vector<int> indicies;
    //     for (const auto& line : mesh.lines) {
    //         indicies.push_back(line[0]);
    //         indicies.push_back(line[1]);
    //     }
    //     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indicies.size(),
    //                  indicies.data(), GL_STATIC_DRAW);

    //     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //     // glUniformMatrix4fv(gWorldID, 1, GL_TRUE, (const
    //     // GLfloat*)TM.GetTSRVC());
    //     glDrawArrays(GL_LINES, 0, indicies.size());
    //     glDisableVertexAttribArray(0);
    // }
    shader_.Use();
    line_->Render();
}