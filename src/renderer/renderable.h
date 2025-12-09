#pragma once
#include <vulkan/vulkan.h>
#include "../engine/transform.h"

struct Renderable
{
    uint16_t index;

    std::vector<VkDescriptorSet> descriptor_sets;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;
};
