#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include "../render_data/vulkan/render_data.h"
#include "../../../external/obj_loader/tiny_obj_loader.h"
enum class OBJ_Mode : uint8_t
{
    Vertex,
    Normal,
    TextureCord,
    Face,
    None
};
namespace model_loader
{
    
    void load_model(const char* model_path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    void parse_obj(const char* path_of_obj, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<std::string> logs);
}/**/