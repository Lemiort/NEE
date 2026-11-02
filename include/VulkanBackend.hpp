#pragma once

#include <volk.h>

#include <cstdint>

struct GLFWwindow;

// Owns Vulkan bootstrap resources, independently of the OpenGL pipelines.
// GLFW must be initialized and the GLFW_NO_API window must outlive this object.
// Call Initialize/Shutdown on the GLFW main thread. Swapchain/rendering comes
// later.
class VulkanBackend {
public:
    VulkanBackend() = default;
    ~VulkanBackend();

    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;
    VulkanBackend(VulkanBackend&&) = delete;
    VulkanBackend& operator=(VulkanBackend&&) = delete;

    // Releases previous resources first; failure leaves the backend empty.
    [[nodiscard]] VkResult Initialize(GLFWwindow& window);
    void Shutdown() noexcept;

    [[nodiscard]] bool IsInitialized() const noexcept;
    // Borrowed handles, valid until Shutdown or the next Initialize call.
    [[nodiscard]] VkInstance GetInstance() const noexcept { return instance_; }
    [[nodiscard]] VkSurfaceKHR GetSurface() const noexcept { return surface_; }
    [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const noexcept {
        return physicalDevice_;
    }
    [[nodiscard]] VkDevice GetDevice() const noexcept { return device_; }
    [[nodiscard]] VkQueue GetGraphicsQueue() const noexcept {
        return graphicsQueue_;
    }
    [[nodiscard]] VkQueue GetPresentQueue() const noexcept {
        return presentQueue_;
    }
    [[nodiscard]] std::uint32_t GetGraphicsQueueFamily() const noexcept {
        return graphicsQueueFamily_;
    }
    [[nodiscard]] std::uint32_t GetPresentQueueFamily() const noexcept {
        return presentQueueFamily_;
    }

private:
    struct QueueFamilies {
        std::uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
        std::uint32_t present = VK_QUEUE_FAMILY_IGNORED;
    };

    [[nodiscard]] VkResult CreateInstance();
    [[nodiscard]] VkResult SelectPhysicalDevice();
    [[nodiscard]] VkResult FindQueueFamilies(VkPhysicalDevice candidate,
                                             QueueFamilies& queues) const;
    [[nodiscard]] VkResult CreateDevice();

    // Per-instance/device dispatch avoids overwriting another backend's
    // functions.
    VolkInstanceTable instanceFunctions_{};
    VolkDeviceTable deviceFunctions_{};
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;
    std::uint32_t graphicsQueueFamily_ = VK_QUEUE_FAMILY_IGNORED;
    std::uint32_t presentQueueFamily_ = VK_QUEUE_FAMILY_IGNORED;
};
