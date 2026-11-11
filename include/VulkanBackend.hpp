#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory>

struct GLFWwindow;

// Owns Vulkan bootstrap resources through Vulkan-Hpp's vk::raii wrappers.
// GLFW must be initialized and the GLFW_NO_API window must outlive this object.
// Call Initialize/Shutdown on the GLFW main thread.
class VulkanBackend {
public:
    VulkanBackend();
    ~VulkanBackend();

    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;
    VulkanBackend(VulkanBackend&&) = delete;
    VulkanBackend& operator=(VulkanBackend&&) = delete;

    // Releases previous resources first; a returned failure leaves the backend
    // empty.
    [[nodiscard]] VkResult Initialize(GLFWwindow& window);
    void Shutdown() noexcept;
    [[nodiscard]] bool IsInitialized() const noexcept;
    // Borrowed C handles for interoperability; never destroy these yourself.
    [[nodiscard]] VkInstance GetInstance() const noexcept;
    [[nodiscard]] VkSurfaceKHR GetSurface() const noexcept;
    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const noexcept;
    [[nodiscard]] VkDevice GetDevice() const noexcept;
    [[nodiscard]] VkQueue GetGraphicsQueue() const noexcept;
    [[nodiscard]] VkQueue GetPresentQueue() const noexcept;
    [[nodiscard]] std::uint32_t GetGraphicsQueueFamily() const noexcept;
    [[nodiscard]] std::uint32_t GetPresentQueueFamily() const noexcept;

private:
    struct Resources;
    std::unique_ptr<Resources> resources_;
};
