#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <filesystem>
#include "../render_data/vulkan/render_data.h"
#include "../../../external/obj_loader/tiny_obj_loader.h"

namespace model_loader
{
    void load_model(const char* model_path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
}/**/