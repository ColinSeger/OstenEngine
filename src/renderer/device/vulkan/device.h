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

QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface);
SwapChainSupportDetails find_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR& surface);

static const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Device
{
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice virtual_device = VK_NULL_HANDLE;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSurfaceKHR surface;
};

void create_device(Device& device,VkInstance& instance, VkSurfaceKHR& surface_reference, const std::vector<const char*>& validation_layers);
// int create_device();

bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface);
bool check_device_extension_support(VkPhysicalDevice device);
void create_virtual_device(Device* device, const std::vector<const char*>& validation_layers);