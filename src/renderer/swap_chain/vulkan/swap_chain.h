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

    VkDevice virtual_device;

    VkCommandPool command_pool;

    std::vector<VkCommandBuffer> command_buffers;

    std::vector<VkImage> swap_chain_images;

    std::vector<VkImageView> swap_chain_image_view;

    std::vector<VkFramebuffer> swap_chain_framebuffers;

    VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilites);

    VkSurfaceFormatKHR select_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);

    VkPresentModeKHR select_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);

    void create_image_views();
    
public:

    SwapChain(GLFWwindow* window, VkPhysicalDevice physical_device, VkSurfaceKHR& surface_reference, VkDevice virtual_device);
    ~SwapChain();

    void create_frame_buffers(VkRenderPass& render_pass);

    void create_command_pool(VkPhysicalDevice physical_device);

    void create_command_buffer(const uint8_t MAX_FRAMES_IN_FLIGHT);

    void record_command_buffer(VkCommandBuffer& command_buffer);

    void start_render_pass(VkCommandBuffer& command_buffer, uint32_t image_index, VkRenderPass render_pass);

    void bind_pipeline(VkCommandBuffer& command_buffer, VkPipeline pipeline, VkBuffer& vertex_buffer, uint32_t vertex_count);

    VkExtent2D get_extent() const {  return screen_extent; }

    VkFormat get_image_format() const { return swap_chain_image_format; }

    std::vector<VkFramebuffer>& get_frame_buffer(){ return swap_chain_framebuffers; }

    
    VkCommandBuffer& get_command_buffer(size_t index) { return command_buffers[index]; }

    VkSwapchainKHR& get_swap_chain() { return swap_chain; }
};

namespace Setup
{
    SwapChainSupportDetails find_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR& surface);

    QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface);
};
