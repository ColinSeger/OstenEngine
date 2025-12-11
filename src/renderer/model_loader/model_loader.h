#pragma once
#include "../../common_includes.h"
#include "../device/vulkan/device.cpp"
#include "../render_data/vulkan/render_data.h"

enum class ObjMode : uint8_t
{
    Vertex,
    Normal,
    TextureCord,
    Face,
    None,
    Comment
};

struct Model
{
    uint32_t index_amount = 0;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;//TODO Look into how to merge into vertex buffer
    VkDeviceMemory index_buffer_memory;
};

namespace ModelLoader
{
    const char valid_chars[14] = "0123456789.-/";

    bool is_valid_char(char c);

    void parse_obj(const char* path_of_obj, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    void serialize(const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    void de_serialize(const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    Model create_model(Device& device, VkCommandPool command_pool, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    Model load_model(Device& device, VkCommandPool command_pool, std::string filename);
}/**/
