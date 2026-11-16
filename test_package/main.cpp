// Compiles against the engine and the libraries it exposes, links them, and
// runs a little engine code that needs no window.
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <glm/vec3.hpp>

#include "FixedStepClock.hpp"

int main() {
    FixedStepClock clock(FixedStepClockSettings{.stepSeconds = 0.1});
    const ClockAdvance advance = clock.Advance(0.25);
    const glm::vec3 up{0.0F, 1.0F, 0.0F};
    spdlog::info("NEE package works: {} steps, ImGui {}, GLFW {}.{}, up.y {}",
                 advance.steps, IMGUI_VERSION, GLFW_VERSION_MAJOR,
                 GLFW_VERSION_MINOR, up.y);
    return advance.steps == 2 ? 0 : 1;
}
