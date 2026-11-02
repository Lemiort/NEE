#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <cstdlib>
#include <memory>

#include "VulkanBackend.hpp"

int main() {
    glfwSetErrorCallback([](int error, const char* description) {
        spdlog::error("GLFW {}: {}", error, description);
    });
    if (glfwInit() != GLFW_TRUE) {
        return EXIT_FAILURE;
    }
    int result = EXIT_FAILURE;
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        const std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window(
            glfwCreateWindow(640, 480, "NEE Vulkan bootstrap", nullptr,
                             nullptr),
            &glfwDestroyWindow);
        if (window) {
            VulkanBackend backend;
            if (backend.Initialize(*window) == VK_SUCCESS &&
                backend.IsInitialized() &&
                backend.GetGraphicsQueue() != VK_NULL_HANDLE &&
                backend.GetPresentQueue() != VK_NULL_HANDLE) {
                // Exercise replacement of live resources, then idempotent
                // teardown.
                if (backend.Initialize(*window) == VK_SUCCESS) {
                    backend.Shutdown();
                    backend.Shutdown();
                    if (!backend.IsInitialized() &&
                        backend.GetInstance() == VK_NULL_HANDLE &&
                        backend.GetSurface() == VK_NULL_HANDLE &&
                        backend.GetPhysicalDevice() == VK_NULL_HANDLE &&
                        backend.GetGraphicsQueue() == VK_NULL_HANDLE &&
                        backend.GetPresentQueue() == VK_NULL_HANDLE) {
                        spdlog::info("Vulkan bootstrap smoke test passed");
                        result = EXIT_SUCCESS;
                    }
                }
            }
        }
    }  // Destroy Vulkan resources and the window before terminating GLFW.
    glfwTerminate();
    return result;
}
