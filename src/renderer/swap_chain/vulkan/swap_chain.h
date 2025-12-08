#pragma once
#include "../../../common_includes.h"
#include "../../device/vulkan/device.h"
#include "../../texture/vulkan/texture.h"

struct RenderBuffer
{
    VkBuffer& vertex_buffer;
    VkBuffer& index_buffer;
};

static VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilites, GLFWwindow* window);

static VkSurfaceFormatKHR select_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);

static VkPresentModeKHR select_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);



struct SwapChain
{
    VkExtent2D screen_extent;
    
    VkSwapchainKHR swap_chain;

    VkFormat swap_chain_image_format;
};



struct SwapChainImages
{
    VkImage depth_image;

    VkDeviceMemory depth_image_memory;

    VkImageView depth_image_view;

    std::vector<VkImage> swap_chain_images;

    std::vector<VkImageView> swap_chain_image_view;

    std::vector<VkFramebuffer> swap_chain_framebuffers;
};
void create_swap_chain(Device& device, WindowSize window, VkSurfaceKHR surface, SwapChain& swap_chain);

int clean_swap_chain(VkDevice& virtual_device, SwapChain& swap_chain, SwapChainImages& swap_chain_images);

static void create_image_views(SwapChainImages& swap_images, VkDevice virtual_device, VkFormat image_format);

void create_swap_chain_images(Device& device, SwapChain& swap_chain,  VkSurfaceKHR surface, SwapChainImages& swap_image);

void create_frame_buffers(SwapChainImages& swap_images, VkDevice virtual_device, VkRenderPass& render_pass, VkImageView depth_image_view, VkExtent2D extent);

void bind_pipeline(VkCommandBuffer& command_buffer, VkPipeline pipeline, VkExtent2D extent);

// int recreate_swap_chain(GLFWwindow* window, Device* device, VkSurfaceKHR surface_reference, SwapChainImages* swap_chain);

VkImageView create_depth_resources(Device& device, VkExtent2D image_size, VkDeviceMemory& depth_image_memory, VkImage& depth_image);

namespace RenderPass
{
    void start_render_pass(VkCommandBuffer& command_buffer, VkFramebuffer frame_buffer, VkRenderPass render_pass, VkExtent2D viewport_extent);
    void end_render_pass(VkCommandBuffer& command_buffer);
}
