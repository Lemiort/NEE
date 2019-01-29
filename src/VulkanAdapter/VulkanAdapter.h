#ifndef VULKANADAPTERVULKANADAPTER_H
#define VULKANADAPTERVULKANADAPTER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>

class VulkanAdapter final {
public:
    using vkCreateDeviceFunction = std::function<VkResult(
        VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)>;
    VulkanAdapter();
    virtual ~VulkanAdapter();

    vkCreateDeviceFunction CreateDevice;
    const char** GetExtensions();
    uint32_t GetExtensionsCount();

    VkInstance GetVkInstance();

private:
    using vkCreateInstanceFunction = std::function<VkResult(
        const VkInstanceCreateInfo* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)>;
    using vkDestroyIstanceFunction = std::function<void(
        VkInstance instance, const VkAllocationCallbacks* pAllocator)>;
    vkCreateInstanceFunction CreateInstance;
    vkDestroyIstanceFunction DestroyInstance;
    const char** extensions_;
    uint32_t extensions_count_;

    VkInstance vk_instance_;
    VkInstanceCreateInfo instance_create_info_ = {};
};

#endif /* VULKANADAPTERVULKANADAPTER_H */
