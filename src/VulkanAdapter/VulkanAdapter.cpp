#include "VulkanAdapter.h"

VulkanAdapter::VulkanAdapter() {
    extensions_ = glfwGetRequiredInstanceExtensions(&extensions_count_);

    PFN_vkCreateInstance pfnCreateInstance =
        reinterpret_cast<PFN_vkCreateInstance>(
            glfwGetInstanceProcAddress(NULL, "vkCreateInstance"));
    CreateInstance = static_cast<vkCreateInstanceFunction>(pfnCreateInstance);

    instance_create_info_.enabledExtensionCount = extensions_count_;
    instance_create_info_.ppEnabledExtensionNames = extensions_;

    auto resullt =
        CreateInstance(&instance_create_info_, nullptr, &vk_instance_);
    if (resullt > 0) {
        throw std::runtime_error("Cannot create vk instance");
    }

    PFN_vkDestroyInstance pfnDestroyInstance =
        reinterpret_cast<PFN_vkDestroyInstance>(
            glfwGetInstanceProcAddress(vk_instance_, "vkDestroyInstance"));
    DestroyInstance = static_cast<vkDestroyIstanceFunction>(pfnDestroyInstance);

    PFN_vkCreateDevice pfnCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(
        glfwGetInstanceProcAddress(vk_instance_, "vkCreateDevice"));
    CreateDevice = static_cast<vkCreateDeviceFunction>(pfnCreateDevice);
}

VulkanAdapter::~VulkanAdapter() { DestroyInstance(vk_instance_, nullptr); }

VkInstance VulkanAdapter::GetVkInstance() { return vk_instance_; }

const char** VulkanAdapter::GetExtensions() { return extensions_; }
uint32_t VulkanAdapter::GetExtensionsCount() { return extensions_count_; }