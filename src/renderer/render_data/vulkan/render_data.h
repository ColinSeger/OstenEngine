#pragma once
#include <array>
#include <cassert>
#include <cstring>
#include "vulkan/vulkan.h"
#include "../../../../external/glm/glm.hpp"
#include "../../device/vulkan/device.h"

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

void create_descriptor_set_layout(VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout);

typedef struct{
    VkVertexInputAttributeDescription array[3];
}VertexAtributes;

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texture_cord;

    static VkVertexInputBindingDescription get_binding_description() {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }


    static VertexAtributes get_attribute_descriptions() {
        VertexAtributes attribute_descriptions{};
        attribute_descriptions.array[0].binding = 0;
        attribute_descriptions.array[0].location = 0;
        attribute_descriptions.array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions.array[0].offset = offsetof(Vertex, position);

        attribute_descriptions.array[1].binding = 0;
        attribute_descriptions.array[1].location = 1;
        attribute_descriptions.array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions.array[1].offset = offsetof(Vertex, color);

        attribute_descriptions.array[2].binding = 0;
        attribute_descriptions.array[2].location = 2;
        attribute_descriptions.array[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions.array[2].offset = offsetof(Vertex, texture_cord);

        return attribute_descriptions;
    }  
};

namespace VertexFunctions{

    void create_buffer(Device* device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);

    void create_vertex_buffer(Device* device, std::vector<Vertex>& vertices, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory, VkCommandPool& command_pool);

    void create_index_buffer(Device* device, std::vector<uint32_t>& indicies, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory, VkCommandPool& command_pool);

    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

    inline VkVertexInputBindingDescription get_binding_description();

    inline std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions();
}

namespace CommandBuffer
{
    VkCommandBuffer begin_single_time_commands(VkDevice virtual_device, VkCommandPool& command_pool);

    void end_single_time_commands(VkDevice virtual_device, VkCommandPool& command_pool, VkQueue graphics_queue, VkCommandBuffer& command_buffer);

    void copy_buffer(Device* device, VkBuffer& src_buffer, VkBuffer& dst_buffer, VkDeviceSize& size, VkCommandPool& command_pool);
}