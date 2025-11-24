#pragma once
#include <vulkan/vulkan.h>
#include <cassert>
#include <vector>
#include <optional>
#include <set>
#include <string>
#include "../../swap_chain/vulkan/swap_chain.h"//Look into removing later


static const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};





struct Device
{
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice virtual_device = VK_NULL_HANDLE;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSurfaceKHR& surface;

    Device(VkInstance& instance, VkSurfaceKHR& surface, const std::vector<const char*>& validation_layers);
    ~Device();
};

namespace DeviceFunctions{
    bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool check_device_extension_support(VkPhysicalDevice device);
    void create_virtual_device(Device* device, const std::vector<const char*>& validation_layers);
}