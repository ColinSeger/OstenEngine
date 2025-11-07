#pragma once
#include <array>
#include <cassert>
#include "vulkan/vulkan.h"
#include "../../../../external/glm/glm.hpp"
#include "../../device/vulkan/device.h"



struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription get_binding_description() {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }


    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, position);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        return attribute_descriptions;
    }  
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

namespace VertexFunctions{

    void copy_buffer(Device* device, VkBuffer& src_buffer, VkBuffer& dst_buffer, VkDeviceSize& size, VkCommandPool& command_pool);

    void create_buffer(Device* device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);

    void create_vertex_buffer(Device* device, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory, VkCommandPool& command_pool);

    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

    inline VkVertexInputBindingDescription get_binding_description();

    inline std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions();
}