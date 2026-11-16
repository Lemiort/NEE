#ifndef IMGUI_OVERLAY_HPP_INCLUDED
#define IMGUI_OVERLAY_HPP_INCLUDED

#include <string>

struct GLFWwindow;
struct ImGuiContext;

struct ImGuiOverlaySettings {
    // File that keeps window positions and sizes between runs, relative to
    // the working directory. Empty keeps nothing on disk.
    std::string iniFile;
};

// Dear ImGui drawn on top of the engine's frames, using the GLFW and
// OpenGL 3 backends that ship with ImGui.
//
// Create it after the application has installed its own GLFW callbacks:
// ImGui installs its callbacks on top and forwards every event to the
// previous ones, and restores them when the overlay is destroyed. Both
// creation and destruction need the window's OpenGL context to be current.
//
// Each frame: BeginFrame(), then any ImGui calls (see ImGuiScopes.hpp), then
// Render() after the scene so the UI is drawn on top.
class ImGuiOverlay {
public:
    // Throws std::logic_error if another overlay exists (ImGui has one
    // current context per thread) and std::runtime_error if a backend
    // cannot start.
    explicit ImGuiOverlay(GLFWwindow& window,
                          ImGuiOverlaySettings settings = {});
    ~ImGuiOverlay();
    ImGuiOverlay(const ImGuiOverlay&) = delete;
    ImGuiOverlay& operator=(const ImGuiOverlay&) = delete;
    ImGuiOverlay(ImGuiOverlay&&) = delete;
    ImGuiOverlay& operator=(ImGuiOverlay&&) = delete;

    // Starts a UI frame. A frame begun but never rendered is ended here.
    void BeginFrame();
    // Draws the UI into the bound draw framebuffer. Does nothing without a
    // begun frame. Restores the OpenGL state it changes.
    void Render();

    // True while ImGui uses the mouse or keyboard, for example when the
    // cursor is over a window; the application should then ignore that
    // input so clicks and keys do not also reach the scene.
    [[nodiscard]] bool WantsMouse() const noexcept;
    [[nodiscard]] bool WantsKeyboard() const noexcept;

private:
    ImGuiContext* context_{};
    // ImGui keeps a pointer to the file name, so it must outlive the context.
    std::string iniFile_;
    bool frameStarted_{false};
};

#endif  // IMGUI_OVERLAY_HPP_INCLUDED
