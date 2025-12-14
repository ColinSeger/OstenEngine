#ifndef DESCRIPTORSETS
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>
#include "../../../external/imgui_test/imgui_impl_vulkan.h"
#include "../../../external/math_3d.h"

const uint8_t FRAMES = 2;

struct Renderable
{
    uint16_t transform_index = 0;
    uint16_t model_index = 0;
    uint16_t texture_index = 0;

    std::vector<VkDescriptorSet> descriptor_sets;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;
};

struct UniformBufferObject {
     mat4_t model;
     mat4_t view;
     mat4_t projection;
};

void create_descriptor_pool(VkDescriptorPool& result, VkDevice virtual_device)
{
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000},
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);

    if(vkCreateDescriptorPool(virtual_device, &pool_info, nullptr, &result) != VK_SUCCESS){
        throw("Descriptor fail");
    }
}

void create_descriptor_set(VkDevice virtual_device, Renderable& render_this, VkDescriptorPool& descriptor_pool, VkDescriptorSetLayout& descriptor_set_layout, VkImageView image_view, VkSampler sampler) {
    std::vector<VkDescriptorSetLayout> layouts(FRAMES, descriptor_set_layout);//Swap for normal array later?

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES);
    allocInfo.pSetLayouts = layouts.data();

    render_this.descriptor_sets.resize(FRAMES);

    if(vkAllocateDescriptorSets(virtual_device, &allocInfo, render_this.descriptor_sets.data()) != VK_SUCCESS){
        throw("Failed to create descriptor sets");
    }

    for (size_t i = 0; i < FRAMES; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = render_this.uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = image_view;
        image_info.sampler = sampler;

        const uint8_t descriptor_size = 2;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = render_this.descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;
        descriptor_writes[0].pImageInfo = nullptr; // Optional
        descriptor_writes[0].pTexelBufferView = nullptr; // Optional

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = render_this.descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
}

void create_descriptor_set_layout(VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout)
{
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { ubo_layout_binding, sampler_layout_binding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = sizeof(bindings) / sizeof(bindings[0]);//Gets size of array
    layoutInfo.pBindings = bindings;

    VkResult result = vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, &descriptor_set_layout);
    if(result != VK_SUCCESS)
    {
        throw("Failed to create descriptor layout");
    }
}

static void create_descriptor_sets(VkDescriptorPool& descriptor_pool, VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout, VkImageView image_view, VkSampler sampler, std::vector<Renderable>& to_render)
{
    for (size_t render_index = 0; render_index < to_render.size(); render_index++)
    {
        Renderable& render_this = to_render[render_index];

        std::vector<VkDescriptorSetLayout> layouts(FRAMES, descriptor_set_layout);//Swap for normal array later?
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptor_pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES);
        allocInfo.pSetLayouts = layouts.data();

        render_this.descriptor_sets.resize(FRAMES);

        if(vkAllocateDescriptorSets(virtual_device, &allocInfo, render_this.descriptor_sets.data()) != VK_SUCCESS){
            throw("Failed to allocate descriptor sets");
        }

        for (size_t i = 0; i < FRAMES; i++) {
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = render_this.uniform_buffers[i];
            buffer_info.offset = 0;
            buffer_info.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo image_info{};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = image_view;
            image_info.sampler = sampler;

            const uint8_t descriptor_size = 2;
            VkWriteDescriptorSet descriptor_writes[descriptor_size]{};
            descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[0].dstSet = render_this.descriptor_sets[i];
            descriptor_writes[0].dstBinding = 0;
            descriptor_writes[0].dstArrayElement = 0;
            descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_writes[0].descriptorCount = 1;
            descriptor_writes[0].pBufferInfo = &buffer_info;
            descriptor_writes[0].pImageInfo = nullptr; // Optional
            descriptor_writes[0].pTexelBufferView = nullptr; // Optional

            descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[1].dstSet = render_this.descriptor_sets[i];
            descriptor_writes[1].dstBinding = 1;
            descriptor_writes[1].dstArrayElement = 0;
            descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_writes[1].descriptorCount = 1;
            descriptor_writes[1].pImageInfo = &image_info;

            vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
        }
    }
}

#endif
#define  DESCRIPTORSETS
