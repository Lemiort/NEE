#include "ImGuiOverlay.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace {
// The engine requires OpenGL 4.6 core.
constexpr const char* kGlslVersion = "#version 460";
bool overlayExists = false;
}  // namespace

ImGuiOverlay::ImGuiOverlay(GLFWwindow& window, ImGuiOverlaySettings settings)
    : iniFile_(std::move(settings.iniFile)) {
    if (overlayExists) {
        throw std::logic_error("Only one ImGuiOverlay may exist at a time");
    }
    IMGUI_CHECKVERSION();
    context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(context_);
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = iniFile_.empty() ? nullptr : iniFile_.c_str();
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(&window, true)) {
        ImGui::DestroyContext(context_);
        throw std::runtime_error("Unable to start the ImGui GLFW backend");
    }
    if (!ImGui_ImplOpenGL3_Init(kGlslVersion)) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(context_);
        throw std::runtime_error("Unable to start the ImGui OpenGL backend");
    }
    overlayExists = true;
}

ImGuiOverlay::~ImGuiOverlay() {
    ImGui::SetCurrentContext(context_);
    if (frameStarted_) {
        ImGui::EndFrame();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(context_);
    overlayExists = false;
}

void ImGuiOverlay::BeginFrame() {
    ImGui::SetCurrentContext(context_);
    if (frameStarted_) {
        ImGui::EndFrame();
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    frameStarted_ = true;
}

void ImGuiOverlay::Render() {
    if (!frameStarted_) {
        return;
    }
    ImGui::SetCurrentContext(context_);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    frameStarted_ = false;
}

bool ImGuiOverlay::WantsMouse() const noexcept {
    return ImGui::GetCurrentContext() == context_ &&
           ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiOverlay::WantsKeyboard() const noexcept {
    return ImGui::GetCurrentContext() == context_ &&
           ImGui::GetIO().WantCaptureKeyboard;
}
