#pragma once
#include "../../../common_includes.h"
#include <cassert>
#include <cstring>
#include "../../device/vulkan/device.h"

enum Direction : uint8_t{
    X = 0,
    Y = 1,
    Z = 2
};

struct Vector2{
    float x = 0;
    float y = 0;
};

struct WindowSize{
    int32_t x = 0;
    int32_t y = 0;
};

struct Vector3{
    float x = 0;
    float y = 0;
    float z = 0;

    Vector3 operator+(const Vector3 add){
        return {
            x + add.x,
            y + add.y,
            z + add.z
        };
    }
    Vector3& operator-=(const Vector3 subtract){
        x -= subtract.x;
        y -= subtract.y;
        z -= subtract.z;
        return *this;
    }
    Vector3& operator+=(const Vector3 add){
        x += add.x;
        y += add.y;
        z += add.z;
        return *this;
    }
    Vector3& operator*(const float mul){
        x *= mul;
        y *= mul;
        z *= mul;
        return *this;
    }
};

struct Matrix {
    float translation[4];
    float rotation[4];
    float scale[4];
};

struct UniformBufferObject {
    mat4_t model;
    mat4_t view;
    mat4_t proj;
};


struct Vertex {
    Vector3 position{};
    vec3_t color{};
    Vector2 texture_cord{};
};

struct VertexAtributes{
    VkVertexInputAttributeDescription array[3];
};

VkVertexInputBindingDescription get_binding_description();
VertexAtributes get_attribute_descriptions();

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
