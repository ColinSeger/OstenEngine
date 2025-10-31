#pragma once
#include <vulkan/vulkan.h>
#include <cassert>
#include <vector>
#include <optional>
#include <set>
#include <string>

struct QueueFamilyIndicies{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> surface_present_modes;
};

class Device
{
private:
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    VkDevice virtual_device;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSurfaceKHR& surface;

    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    void create_virtual_device(bool enable_validation);

    bool is_device_suitable(VkPhysicalDevice device);

    bool check_device_extension_support(VkPhysicalDevice device);

    QueueFamilyIndicies find_queue_families(VkPhysicalDevice device);

    SwapChainSupportDetails find_swap_chain_support(VkPhysicalDevice device);


public:
    Device(VkInstance& instance, VkSurfaceKHR& surface, bool enable_validation);
    ~Device();
};