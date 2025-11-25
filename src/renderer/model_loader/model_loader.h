#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include "../render_data/vulkan/render_data.h"

enum class OBJ_Mode : uint8_t
{
    Vertex,
    Normal,
    TextureCord,
    Face,
    None,
    Comment
};
namespace model_loader
{

    void parse_obj(const char* path_of_obj, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<std::string> logs);
}/**/