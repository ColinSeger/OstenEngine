#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include "../../additional_things/arena.h"

typedef struct
{
    size_t arena_index;
    size_t amount;
} ShaderMemoryIndexing;

static inline ShaderMemoryIndexing load_shader(const char* file_name, MemArena& memory_arena)
{
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        throw "Failed to load shaders";
    }

    ShaderMemoryIndexing result;
    result.amount = (size_t) file.tellg();

    result.arena_index = arena_alloc_memory(memory_arena, sizeof(char) * result.amount);

    file.seekg(0);
    file.read((char*)memory_arena[result.arena_index], result.amount);

    file.close();
    return result;
}

static inline VkShaderModule create_shader(const ShaderMemoryIndexing& code, VkDevice virtual_device, MemArena& memory_arena) {
    VkShaderModule shader_result;
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.amount;
    create_info.pCode = reinterpret_cast<const uint32_t*>(memory_arena[code.arena_index]);

    if (vkCreateShaderModule(virtual_device, &create_info, nullptr, &shader_result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shader_result;
}
