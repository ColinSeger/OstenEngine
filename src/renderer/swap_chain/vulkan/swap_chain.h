#pragma once
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdint.h>
#include <optional>
#include "../../device/vulkan/device.h"
#include "../../../../external/glm/glm.hpp"

struct QueueFamilyIndicies{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> surface_present_modes;
};

class SwapChain
{
private:

    VkSwapchainKHR swap_chain;

    VkExtent2D screen_extent;

    VkFormat swap_chain_image_format;

    GLFWwindow* main_window = nullptr;
    
    VkSurfaceKHR& surface;

    VkDevice& virtual_device;

    std::vector<VkImage> swap_chain_images;

    std::vector<VkImageView> swap_chan_image_view;

    VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilites);

    VkSurfaceFormatKHR select_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);

    VkPresentModeKHR select_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);

    void create_image_views();
    
public:

    SwapChain(GLFWwindow* window, VkPhysicalDevice physical_device, VkSurfaceKHR& surface_reference, VkDevice virtual_device);
    ~SwapChain();
    
};

namespace Setup
{
    SwapChainSupportDetails find_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR& surface);

    QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface);
};
