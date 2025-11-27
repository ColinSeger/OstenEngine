#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stdint.h>
#include <optional>
#include "../../../../external/imgui_test/imgui_impl_glfw.h"
#include "../../../../external/imgui_test/imgui_impl_vulkan.h"
#include "../../../../external/glm/glm.hpp"
#include "../../device/vulkan/device.h"

struct RenderBuffer
{
    VkBuffer& vertex_buffer;
    VkBuffer& index_buffer;
};

VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilites, GLFWwindow* window);

VkSurfaceFormatKHR select_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);

VkPresentModeKHR select_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);

struct SwapChain
{
    VkSwapchainKHR swap_chain;

    VkExtent2D screen_extent;

    VkFormat swap_chain_image_format;
};

SwapChain create_swap_chain(GLFWwindow* window, Device* device, VkSurfaceKHR surface);

struct SwapChainImages
{
    std::vector<VkImage> swap_chain_images;

    std::vector<VkImageView> swap_chain_image_view;

    std::vector<VkFramebuffer> swap_chain_framebuffers;

    void create_image_views(VkDevice virtual_device, VkFormat image_format);

    SwapChainImages(SwapChain* swap_chain, Device* device, VkSurfaceKHR surface_reference);
    ~SwapChainImages();

    void create_frame_buffers(VkDevice virtual_device, VkRenderPass& render_pass, VkImageView depth_image_view, VkExtent2D extent);

    void bind_pipeline(VkCommandBuffer& command_buffer, VkPipeline pipeline, VkExtent2D extent);
};

int recreate_swap_chain(GLFWwindow* window, Device* device, VkSurfaceKHR surface_reference, SwapChainImages* swap_chain);

namespace RenderPass
{
    void start_render_pass(VkCommandBuffer& command_buffer, VkFramebuffer frame_buffer, VkRenderPass render_pass, VkExtent2D viewport_extent);
    void end_render_pass(VkCommandBuffer& command_buffer);
}