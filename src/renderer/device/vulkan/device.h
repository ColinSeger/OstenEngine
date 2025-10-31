#pragma once
#include <vulkan/vulkan.h>
#include <cassert>
#include <vector>
#include <optional>
#include <set>
#include <string>
#include "../../swap_chain/vulkan/swap_chain.h"

class Device
{
private:
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    VkDevice virtual_device;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSurfaceKHR& surface;

    // SwapChain* swap_chain = nullptr;

    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    void create_virtual_device(bool enable_validation);

    bool is_device_suitable(VkPhysicalDevice device);

    bool check_device_extension_support(VkPhysicalDevice device);

public:
    Device(VkInstance& instance, VkSurfaceKHR& surface, bool enable_validation);
    ~Device();
};