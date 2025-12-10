#pragma once
#include "../common_includes.h"

struct Renderable
{
    uint16_t transform_index;

    std::vector<VkDescriptorSet> descriptor_sets;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;
};
