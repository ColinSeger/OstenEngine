#pragma once
#include "../../../common_includes.h"
#include <set>

struct optional
{
    uint32_t number;
    bool has_value = false;
};

struct QueueFamilyIndicies{
    optional graphics_family;
    optional present_family;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> surface_present_modes;
};

static const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Device
{
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice virtual_device = VK_NULL_HANDLE;

    VkQueue graphics_queue;
    VkQueue present_queue;
};

void create_device(Device& device,VkInstance& instance, VkSurfaceKHR& surface_reference, const std::vector<const char*>& validation_layers);
// int create_device();

void destroy_device(Device& device);

static bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface);
static bool check_device_extension_support(VkPhysicalDevice device);
static void create_virtual_device(Device& device, VkSurfaceKHR surface, const std::vector<const char*>& validation_layers);

QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface);
SwapChainSupportDetails find_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR& surface);
