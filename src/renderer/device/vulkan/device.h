#pragma once
#include "../../../common_includes.h"
#include "../../render_data/vulkan/render_data.h"
#include <string.h>
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



namespace CommandBuffer
{
    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

    VkCommandBuffer begin_single_time_commands(VkDevice virtual_device, VkCommandPool& command_pool);

    void end_single_time_commands(VkDevice virtual_device, VkCommandPool& command_pool, VkQueue graphics_queue, VkCommandBuffer& command_buffer);

    void copy_buffer(Device& device, VkBuffer& src_buffer, VkBuffer& dst_buffer, VkDeviceSize& size, VkCommandPool& command_pool);

    void create_buffer(Device& device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);

    void create_vertex_buffer(Device& device, std::vector<Vertex>& vertices, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory, VkCommandPool& command_pool);

    void create_index_buffer(Device& device, std::vector<uint32_t>& indicies, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory, VkCommandPool& command_pool);

    void record_command_buffer(VkCommandBuffer& command_buffer);

    void create_command_buffers(std::vector<VkCommandBuffer>& command_buffers, VkDevice virtual_device, VkCommandPool& command_pool, const uint8_t MAX_FRAMES_IN_FLIGHT);

    VkCommandPool create_command_pool(Device& device, VkSurfaceKHR surface);
}
