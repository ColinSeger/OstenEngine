#pragma once
#include <vulkan/vulkan.h>
#include <fstream>
#include "../../additional_things/arena.h"
#include "vulkan/vulkan_core.h"

typedef struct
{
    size_t arena_index;
    size_t amount;
} ShaderMemoryIndexing;

static inline ShaderMemoryIndexing load_shader(const char* file_name, HeapStack* heap_stack)
{
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        throw "Failed to load shaders";
    }

    ShaderMemoryIndexing result;
    result.amount = (size_t) file.tellg();

    result.arena_index = arena_alloc_memory(heap_stack, sizeof(char) * result.amount);

    file.seekg(0);

    void* memory = get_at_index(heap_stack, result.arena_index);
    file.read((char*)memory, result.amount);

    file.close();
    return result;
}

static inline VkPipelineShaderStageCreateInfo create_shader(const ShaderMemoryIndexing& code,const VkShaderStageFlagBits shader_stages, VkDevice virtual_device, HeapStack* heap_stack) {
    VkShaderModule shader_result;
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.amount;
    create_info.pCode = (uint32_t*)get_at_index(heap_stack, code.arena_index);

    if (vkCreateShaderModule(virtual_device, &create_info, nullptr, &shader_result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    VkPipelineShaderStageCreateInfo vertex_stage_info{};
    vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage_info.flags = 0;
    vertex_stage_info.stage = shader_stages;
    vertex_stage_info.module = shader_result;
    vertex_stage_info.pName = "main";

    return vertex_stage_info;
}
