#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stdint.h>
#include <optional>
#include "../../../external/imgui/backends/imgui_impl_glfw.h"
#include "../../../external/imgui/backends/imgui_impl_vulkan.h"
#include "../../../../external/glm/glm.hpp"
#include "../../device/vulkan/device.h"
// #include "../../texture/vulkan/texture.h"

struct QueueFamilyIndicies{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> surface_formats;
    std::vector<VkPresentModeKHR> surface_present_modes;
};

struct RenderBuffer
{
    VkBuffer& vertex_buffer;
    VkBuffer& index_buffer;
};

class SwapChain
{
private:

    VkSwapchainKHR swap_chain;

    VkExtent2D screen_extent;

    VkFormat swap_chain_image_format;
    
    VkSurfaceKHR& surface;

    VkDevice virtual_device;

    VkCommandPool command_pool;

    std::vector<VkImage> swap_chain_images;

    std::vector<VkImageView> swap_chain_image_view;

    std::vector<VkFramebuffer> swap_chain_framebuffers;

    //VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilites, GLFWwindow* window);

    VkSurfaceFormatKHR select_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);

    VkPresentModeKHR select_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);

    void create_image_views();
    
public:

    SwapChain(GLFWwindow* window, VkPhysicalDevice physical_device, VkSurfaceKHR& surface_reference, VkDevice virtual_device);
    ~SwapChain();

    void create_frame_buffers(VkRenderPass& render_pass, VkImageView depth_image_view);

    void create_command_pool(VkPhysicalDevice physical_device);

    void create_command_buffer(const uint8_t MAX_FRAMES_IN_FLIGHT);

    void start_render_pass(VkCommandBuffer& command_buffer, uint32_t image_index, VkRenderPass render_pass);

    void bind_pipeline(VkCommandBuffer& command_buffer, VkPipeline pipeline, VkPipelineLayout pipeline_layout, std::vector<VkDescriptorSet> descriptor_set, RenderBuffer& render_buffer, uint32_t vertex_count, const uint32_t index_amount, uint8_t frame);

    VkExtent2D get_extent() const {  return screen_extent; }

    VkFormat get_image_format() const { return swap_chain_image_format; }

    // std::vector<VkFramebuffer>& get_frame_buffer(){ return swap_chain_framebuffers; }

    
    // VkCommandBuffer& get_command_buffer(size_t index) { return command_buffers[index]; }

    VkSwapchainKHR& get_swap_chain() { return swap_chain; }

    VkCommandPool& get_command_pool() {  return command_pool; }
};

namespace Setup
{
    SwapChainSupportDetails find_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR& surface);

    QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface);

    VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilites, GLFWwindow* window);
};
