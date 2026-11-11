#include "VulkanBackend.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <iterator>
#include <memory>
#include <new>
#include <string_view>
#include <utility>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

struct VulkanBackend::Resources {
    // Reverse destruction order keeps children alive only while parents exist.
    vk::raii::Context context;
    vk::raii::Instance instance{nullptr};
    vk::raii::SurfaceKHR surface{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::Device device{nullptr};
    vk::raii::Queue graphicsQueue{nullptr};
    vk::raii::Queue presentQueue{nullptr};
    std::uint32_t graphicsFamily = VK_QUEUE_FAMILY_IGNORED;
    std::uint32_t presentFamily = VK_QUEUE_FAMILY_IGNORED;

    void CreateInstance();
    [[nodiscard]] bool SelectPhysicalDevice();
    void CreateDevice();
};

void VulkanBackend::Resources::CreateInstance() {
    std::uint32_t extensionCount = 0;
    const char* const* extensions =
        glfwGetRequiredInstanceExtensions(&extensionCount);
    if (extensions == nullptr || extensionCount == 0) {
        throw vk::ExtensionNotPresentError(
            "GLFW cannot find Vulkan window surface extensions");
    }
    vk::ApplicationInfo applicationInfo;
    applicationInfo.pApplicationName = "NEE";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.pEngineName = "NEE";
    applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;
    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    instance = vk::raii::Instance(context, createInfo);
}

bool VulkanBackend::Resources::SelectPhysicalDevice() {
    for (auto& candidate : instance.enumeratePhysicalDevices()) {
        const auto extensions = candidate.enumerateDeviceExtensionProperties();
        const bool hasSwapchain =
            std::ranges::any_of(extensions, [](const auto& extension) {
                return std::string_view{std::data(extension.extensionName)} ==
                       VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            });
        if (!hasSwapchain) {
            continue;
        }
        std::uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
        std::uint32_t present = VK_QUEUE_FAMILY_IGNORED;
        const auto families = candidate.getQueueFamilyProperties();
        for (std::uint32_t index = 0; index < families.size(); ++index) {
            const auto& family = families.at(index);
            if (family.queueCount == 0) {
                continue;
            }
            const bool supportsGraphics = static_cast<bool>(
                family.queueFlags & vk::QueueFlagBits::eGraphics);
            const bool supportsPresent =
                candidate.getSurfaceSupportKHR(index, *surface) == VK_TRUE;
            if (supportsGraphics) {
                graphics = index;
            }
            if (supportsPresent) {
                present = index;
            }
            if (supportsGraphics && supportsPresent) {
                break;
            }
        }
        if (graphics != VK_QUEUE_FAMILY_IGNORED &&
            present != VK_QUEUE_FAMILY_IGNORED) {
            const auto properties = candidate.getProperties();
            spdlog::info(
                "Vulkan device: {} (graphics queue {}, present queue {})",
                std::data(properties.deviceName), graphics, present);
            physicalDevice = std::move(candidate);
            graphicsFamily = graphics;
            presentFamily = present;
            return true;
        }
    }
    return false;
}

void VulkanBackend::Resources::CreateDevice() {
    constexpr float priority = 1.0F;
    const std::array families{graphicsFamily, presentFamily};
    std::array<vk::DeviceQueueCreateInfo, 2> queueInfos{};
    const std::uint32_t queueCount = graphicsFamily == presentFamily ? 1U : 2U;
    for (std::uint32_t index = 0; index < queueCount; ++index) {
        auto& info = queueInfos.at(index);
        info.queueFamilyIndex = families.at(index);
        info.queueCount = 1;
        info.pQueuePriorities = &priority;
    }
    const char* extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    vk::DeviceCreateInfo createInfo;
    createInfo.queueCreateInfoCount = queueCount;
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = &extension;
    device = vk::raii::Device(physicalDevice, createInfo);
    graphicsQueue = device.getQueue(graphicsFamily, 0);
    presentQueue = device.getQueue(presentFamily, 0);
}

VulkanBackend::VulkanBackend() = default;
VulkanBackend::~VulkanBackend() { Shutdown(); }

VkResult VulkanBackend::Initialize(GLFWwindow& window) {
    Shutdown();
    if (glfwGetWindowAttrib(&window, GLFW_CLIENT_API) != GLFW_NO_API) {
        spdlog::error("Vulkan requires a window created with GLFW_NO_API");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    try {
        // Local ownership cleans up partially constructed resources on every
        // failure.
        auto resources = std::make_unique<Resources>();
        resources->CreateInstance();
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        const VkResult result = glfwCreateWindowSurface(
            static_cast<VkInstance>(*resources->instance), &window, nullptr,
            &surface);
        if (result != VK_SUCCESS) {
            spdlog::error("Vulkan window surface creation failed ({})",
                          static_cast<int>(result));
            return result;
        }
        // Adopt GLFW's handle immediately; SurfaceKHR now owns its destruction.
        resources->surface = vk::raii::SurfaceKHR(resources->instance, surface);
        if (!resources->SelectPhysicalDevice()) {
            spdlog::error(
                "No Vulkan device supports graphics, presentation and "
                "VK_KHR_swapchain");
            return VK_ERROR_FEATURE_NOT_PRESENT;
        }
        resources->CreateDevice();
        resources_ = std::move(resources);
        return VK_SUCCESS;
    } catch (const vk::SystemError& error) {
        spdlog::error("Vulkan backend initialization failed: {}", error.what());
        return static_cast<VkResult>(error.code().value());
    } catch (const std::bad_alloc&) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    } catch (const std::exception& error) {
        // Context's dynamic loader throws std::runtime_error if Vulkan is
        // absent.
        spdlog::error("Cannot initialize Vulkan: {}", error.what());
        return VK_ERROR_INITIALIZATION_FAILED;
    }
}

void VulkanBackend::Shutdown() noexcept {
    if (resources_ && *resources_->device) {
        try {
            resources_->device.waitIdle();
        } catch (const vk::SystemError& error) {
            // Device loss must not prevent RAII cleanup or escape the
            // destructor.
            (void)std::fputs(error.what(), stderr);
            (void)std::fputc('\n', stderr);
        }
    }
    resources_.reset();
}

bool VulkanBackend::IsInitialized() const noexcept {
    return resources_ != nullptr;
}
VkInstance VulkanBackend::GetInstance() const noexcept {
    return resources_ ? static_cast<VkInstance>(*resources_->instance)
                      : VK_NULL_HANDLE;
}
VkSurfaceKHR VulkanBackend::GetSurface() const noexcept {
    return resources_ ? static_cast<VkSurfaceKHR>(*resources_->surface)
                      : VK_NULL_HANDLE;
}
VkPhysicalDevice VulkanBackend::GetPhysicalDevice() const noexcept {
    return resources_
               ? static_cast<VkPhysicalDevice>(*resources_->physicalDevice)
               : VK_NULL_HANDLE;
}
VkDevice VulkanBackend::GetDevice() const noexcept {
    return resources_ ? static_cast<VkDevice>(*resources_->device)
                      : VK_NULL_HANDLE;
}
VkQueue VulkanBackend::GetGraphicsQueue() const noexcept {
    return resources_ ? static_cast<VkQueue>(*resources_->graphicsQueue)
                      : VK_NULL_HANDLE;
}
VkQueue VulkanBackend::GetPresentQueue() const noexcept {
    return resources_ ? static_cast<VkQueue>(*resources_->presentQueue)
                      : VK_NULL_HANDLE;
}
std::uint32_t VulkanBackend::GetGraphicsQueueFamily() const noexcept {
    return resources_ ? resources_->graphicsFamily : VK_QUEUE_FAMILY_IGNORED;
}
std::uint32_t VulkanBackend::GetPresentQueueFamily() const noexcept {
    return resources_ ? resources_->presentFamily : VK_QUEUE_FAMILY_IGNORED;
}
