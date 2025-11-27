#pragma once
#include <array>
#include <cassert>
#include <cstring>
#include "vulkan/vulkan.h"
#include "../../../debugger/debugger.h"
#include "../../../../external/glm/glm.hpp"
#include "../../device/vulkan/device.h"

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
struct Vertex {
    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec2 texture_cord{}; 
};

struct VertexAtributes{
    VkVertexInputAttributeDescription array[3];
};

VkVertexInputBindingDescription get_binding_description();
VertexAtributes get_attribute_descriptions();

void create_descriptor_set_layout(VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout);

namespace CommandBuffer
{
    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

    VkCommandBuffer begin_single_time_commands(VkDevice virtual_device, VkCommandPool& command_pool);

    void end_single_time_commands(VkDevice virtual_device, VkCommandPool& command_pool, VkQueue graphics_queue, VkCommandBuffer& command_buffer);

    void copy_buffer(Device* device, VkBuffer& src_buffer, VkBuffer& dst_buffer, VkDeviceSize& size, VkCommandPool& command_pool);
    
    void create_buffer(Device* device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);

    void create_vertex_buffer(Device* device, std::vector<Vertex>& vertices, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory, VkCommandPool& command_pool);

    void create_index_buffer(Device* device, std::vector<uint32_t>& indicies, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory, VkCommandPool& command_pool);

    void record_command_buffer(VkCommandBuffer& command_buffer);

    void create_command_buffers(std::vector<VkCommandBuffer>& command_buffers, VkDevice virtual_device, VkCommandPool& command_pool, const uint8_t MAX_FRAMES_IN_FLIGHT);

    VkCommandPool create_command_pool(Device* device, VkSurfaceKHR surface);
}