#include "VulkanBackend.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <string_view>
#include <vector>

VulkanBackend::~VulkanBackend() { Shutdown(); }

bool VulkanBackend::IsInitialized() const noexcept {
    return device_ != VK_NULL_HANDLE;
}

VkResult VulkanBackend::Initialize(GLFWwindow& window) {
    Shutdown();
    if (glfwGetWindowAttrib(&window, GLFW_CLIENT_API) != GLFW_NO_API) {
        spdlog::error("Vulkan requires a window created with GLFW_NO_API");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Volk's loader is process-wide; repeated initialization would reload its
    // DLL.
    static const VkResult loaderResult = volkInitialize();
    VkResult result = loaderResult;
    if (result != VK_SUCCESS) {
        spdlog::error(
            "Cannot load Vulkan; install a Vulkan-capable GPU driver ({})",
            static_cast<int>(result));
        return result;
    }
    // Use the same system loader for GLFW and our Vulkan calls.
    glfwInitVulkanLoader(vkGetInstanceProcAddr);

    result = CreateInstance();
    if (result == VK_SUCCESS) {
        result =
            glfwCreateWindowSurface(instance_, &window, nullptr, &surface_);
    }
    if (result == VK_SUCCESS) {
        result = SelectPhysicalDevice();
    }
    if (result == VK_SUCCESS) {
        result = CreateDevice();
    }
    if (result != VK_SUCCESS) {
        spdlog::error("Vulkan backend initialization failed (VkResult {})",
                      static_cast<int>(result));
        Shutdown();
    }
    return result;
}

VkResult VulkanBackend::CreateInstance() {
    std::uint32_t extensionCount = 0;
    const char* const* extensions =
        glfwGetRequiredInstanceExtensions(&extensionCount);
    if (extensions == nullptr || extensionCount == 0) {
        spdlog::error("GLFW cannot find Vulkan window surface extensions");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "NEE";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.pEngineName = "NEE";
    applicationInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

    const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
    if (result == VK_SUCCESS) {
        volkLoadInstanceTable(&instanceFunctions_, instance_);
    }
    return result;
}

VkResult VulkanBackend::SelectPhysicalDevice() {
    std::uint32_t deviceCount = 0;
    VkResult result = instanceFunctions_.vkEnumeratePhysicalDevices(
        instance_, &deviceCount, nullptr);
    if (result != VK_SUCCESS) {
        return result;
    }
    if (deviceCount == 0) {
        spdlog::error("No Vulkan physical devices found");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = instanceFunctions_.vkEnumeratePhysicalDevices(
        instance_, &deviceCount, devices.data());
    if (result != VK_SUCCESS) {
        return result;
    }
    devices.resize(deviceCount);

    for (VkPhysicalDevice candidate : devices) {
        std::uint32_t extensionCount = 0;
        result = instanceFunctions_.vkEnumerateDeviceExtensionProperties(
            candidate, nullptr, &extensionCount, nullptr);
        if (result != VK_SUCCESS) {
            return result;
        }
        std::vector<VkExtensionProperties> extensions(extensionCount);
        result = instanceFunctions_.vkEnumerateDeviceExtensionProperties(
            candidate, nullptr, &extensionCount, extensions.data());
        if (result != VK_SUCCESS) {
            return result;
        }
        extensions.resize(extensionCount);
        const bool hasSwapchain =
            std::ranges::any_of(extensions, [](const auto& extension) {
                return std::string_view{std::data(extension.extensionName)} ==
                       VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            });
        if (!hasSwapchain) {
            continue;
        }

        QueueFamilies queues;
        result = FindQueueFamilies(candidate, queues);
        if (result != VK_SUCCESS) {
            return result;
        }
        if (queues.graphics != VK_QUEUE_FAMILY_IGNORED &&
            queues.present != VK_QUEUE_FAMILY_IGNORED) {
            physicalDevice_ = candidate;
            graphicsQueueFamily_ = queues.graphics;
            presentQueueFamily_ = queues.present;
            VkPhysicalDeviceProperties properties{};
            instanceFunctions_.vkGetPhysicalDeviceProperties(candidate,
                                                             &properties);
            spdlog::info(
                "Vulkan device: {} (graphics queue {}, present queue {})",
                properties.deviceName, queues.graphics, queues.present);
            return VK_SUCCESS;
        }
    }
    spdlog::error(
        "No Vulkan device supports graphics, presentation and "
        "VK_KHR_swapchain");
    return VK_ERROR_FEATURE_NOT_PRESENT;
}

VkResult VulkanBackend::FindQueueFamilies(VkPhysicalDevice candidate,
                                          QueueFamilies& queues) const {
    std::uint32_t familyCount = 0;
    instanceFunctions_.vkGetPhysicalDeviceQueueFamilyProperties(
        candidate, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familyCount);
    instanceFunctions_.vkGetPhysicalDeviceQueueFamilyProperties(
        candidate, &familyCount, families.data());
    for (std::uint32_t index = 0; index < familyCount; ++index) {
        const auto& family = families.at(index);
        if (family.queueCount == 0) {
            continue;
        }
        VkBool32 supportsPresent = VK_FALSE;
        const VkResult result =
            instanceFunctions_.vkGetPhysicalDeviceSurfaceSupportKHR(
                candidate, index, surface_, &supportsPresent);
        if (result != VK_SUCCESS) {
            return result;
        }
        const bool supportsGraphics =
            (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
        if (supportsGraphics) {
            queues.graphics = index;
        }
        if (supportsPresent == VK_TRUE) {
            queues.present = index;
        }
        // Prefer one family, but also support separate presentation queues.
        if (supportsGraphics && supportsPresent == VK_TRUE) {
            break;
        }
    }
    return VK_SUCCESS;
}

VkResult VulkanBackend::CreateDevice() {
    constexpr float priority = 1.0F;
    const std::array families{graphicsQueueFamily_, presentQueueFamily_};
    std::array<VkDeviceQueueCreateInfo, 2> queueInfos{};
    const std::uint32_t queueCount =
        graphicsQueueFamily_ == presentQueueFamily_ ? 1U : 2U;
    for (std::uint32_t index = 0; index < queueCount; ++index) {
        auto& queueInfo = queueInfos.at(index);
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = families.at(index);
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;
    }
    const char* extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCount;
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = &extension;
    const VkResult result = instanceFunctions_.vkCreateDevice(
        physicalDevice_, &createInfo, nullptr, &device_);
    if (result == VK_SUCCESS) {
        volkLoadDeviceTable(&deviceFunctions_, device_);
        deviceFunctions_.vkGetDeviceQueue(device_, graphicsQueueFamily_, 0,
                                          &graphicsQueue_);
        deviceFunctions_.vkGetDeviceQueue(device_, presentQueueFamily_, 0,
                                          &presentQueue_);
    }
    return result;
}

void VulkanBackend::Shutdown() noexcept {
    if (device_ != VK_NULL_HANDLE) {
        // Wait for future rendering work before releasing device resources.
        (void)deviceFunctions_.vkDeviceWaitIdle(device_);
        deviceFunctions_.vkDestroyDevice(device_, nullptr);
    }
    if (surface_ != VK_NULL_HANDLE) {
        instanceFunctions_.vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }
    if (instance_ != VK_NULL_HANDLE) {
        instanceFunctions_.vkDestroyInstance(instance_, nullptr);
    }
    device_ = VK_NULL_HANDLE;
    surface_ = VK_NULL_HANDLE;
    instance_ = VK_NULL_HANDLE;
    physicalDevice_ = VK_NULL_HANDLE;
    graphicsQueue_ = VK_NULL_HANDLE;
    presentQueue_ = VK_NULL_HANDLE;
    graphicsQueueFamily_ = VK_QUEUE_FAMILY_IGNORED;
    presentQueueFamily_ = VK_QUEUE_FAMILY_IGNORED;
    instanceFunctions_ = {};
    deviceFunctions_ = {};
}
