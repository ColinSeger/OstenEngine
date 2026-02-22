#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "vulkan/vulkan_core.h"
#include "../../platform.h"

static inline bool create_shader(const FileData code, const VkShaderStageFlagBits shader_stages, VkDevice virtual_device, VkPipelineShaderStageCreateInfo* result) {
    VkShaderModule shader_result = {};
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.file_size;
    create_info.pCode = (uint32_t*)code.file_data;

    if (vkCreateShaderModule(virtual_device, &create_info, nullptr, &shader_result) != VK_SUCCESS) {
        return 0;
    }
    result->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    result->flags = 0;
    result->stage = shader_stages;
    result->module = shader_result;
    result->pName = "main";

    return 1;
}
