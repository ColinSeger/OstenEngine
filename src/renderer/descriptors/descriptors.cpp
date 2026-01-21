#include <cstddef>
#ifndef DESCRIPTORSETS
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../../../external/math_3d.h"
#include "../device/vulkan/device.cpp"

constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;

typedef struct{
    VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSet shadow_sets[MAX_FRAMES_IN_FLIGHT];

    VkBuffer uniform_buffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniform_buffers_memory[MAX_FRAMES_IN_FLIGHT];
    void* uniform_buffers_mapped[MAX_FRAMES_IN_FLIGHT];
} RenderDescriptors;

typedef struct{
    mat4_t model;
    mat4_t view;
    mat4_t projection;
} UniformBufferObject;

void create_descriptor_pool(VkDescriptorPool& result, VkDevice virtual_device, const uint32_t pool_size){
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         MAX_FRAMES_IN_FLIGHT * pool_size},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * pool_size},
        {VK_DESCRIPTOR_TYPE_SAMPLER,                MAX_FRAMES_IN_FLIGHT * pool_size}
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = pool_size;

    if(vkCreateDescriptorPool(virtual_device, &pool_info, nullptr, &result) != VK_SUCCESS){
        throw("Descriptor failed to create");
    }
}

void update_descriptor_set(VkDevice virtual_device, RenderDescriptors& render_this, VkImageView image_view, VkSampler sampler){
    return;
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = render_this.uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        constexpr uint8_t descriptor_size = 2;
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

VkResult create_forward_descriptor_set_layout(VkDevice virtual_device, VkDescriptorSetLayout* descriptor_set_layout){
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 2;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { ubo_layout_binding, sampler_layout_binding };
    constexpr size_t bindings_amount = sizeof(bindings) / sizeof(bindings[0]);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings_amount;
    layoutInfo.pBindings = bindings;

    return vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, descriptor_set_layout);
}

VkResult create_shadow_descriptor_layout(VkDevice virtual_device, VkDescriptorSetLayout* descriptor_set_layout){
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { ubo_layout_binding };
    constexpr size_t bindings_amount = sizeof(bindings) / sizeof(bindings[0]);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings_amount;
    layoutInfo.pBindings = bindings;

    return vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, descriptor_set_layout);
}

VkResult create_shadow_sets(VkDevice virtual_device, RenderDescriptors& render_this, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout)
{
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts;

    VkResult status = vkAllocateDescriptorSets(virtual_device, &allocInfo, render_this.shadow_sets);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = render_this.uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        constexpr uint8_t descriptor_size = 1;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = render_this.shadow_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }

    return status;
}

void create_descriptor_set(VkDevice virtual_device, RenderDescriptors& render_this, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, VkImageView image_view, VkSampler sampler, VkImageView shadow_view, VkSampler shadow_sampler) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts;

    VkResult allocation_status = vkAllocateDescriptorSets(virtual_device, &allocInfo, render_this.descriptor_sets);

    if(allocation_status != VK_SUCCESS)
        throw("Failed to create descriptor sets");

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    VkDescriptorImageInfo shadow_info{};
    shadow_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    shadow_info.imageView = shadow_view;
    shadow_info.sampler = shadow_sampler;

    VkDescriptorImageInfo image_descriptors_info[] = {image_info, shadow_info};

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = render_this.uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        constexpr uint8_t descriptor_size = 2;
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
        descriptor_writes[1].descriptorCount = 2;
        descriptor_writes[1].pImageInfo = image_descriptors_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
}

static void create_descriptor_sets(
        VkDescriptorPool& descriptor_pool,
        VkDevice virtual_device,
        VkDescriptorSetLayout& descriptor_set_layout,
        VkImageView image_view,
        VkSampler sampler,
        RenderDescriptors* render_descriptors,
        uint32_t amount
    ){
    for (size_t render_index = 0; render_index < amount; render_index++)
    {
        RenderDescriptors& render_this = render_descriptors[render_index];

        VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
        for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            layouts[i] = descriptor_set_layout;
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptor_pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts;
        VkResult allocation_status = vkAllocateDescriptorSets(virtual_device, &allocInfo, render_this.descriptor_sets);

        if(allocation_status != VK_SUCCESS)
            throw("Failed to allocate descriptor sets");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer = render_this.uniform_buffers[i];
            buffer_info.offset = 0;
            buffer_info.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo image_info{};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = image_view;
            image_info.sampler = sampler;

            constexpr uint8_t descriptor_size = 2;
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

void create_uniform_buffers(RenderDescriptors* render_descriptors, uint32_t amount, Device device) {
    for (size_t render_index = 0; render_index < amount; render_index++)
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            CommandBuffer::create_buffer(
                device,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                bufferSize,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                render_descriptors[render_index].uniform_buffers[i],
                render_descriptors[render_index].uniform_buffers_memory[i]
            );

            vkMapMemory(device.virtual_device, render_descriptors[render_index].uniform_buffers_memory[i], 0, bufferSize, 0, &render_descriptors[render_index].uniform_buffers_mapped[i]);
        }
    }
}

void create_uniform_buffer(RenderDescriptors& render_descriptor, Device& device) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CommandBuffer::create_buffer(
            device,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            bufferSize,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            render_descriptor.uniform_buffers[i],
            render_descriptor.uniform_buffers_memory[i]
        );

        vkMapMemory(device.virtual_device, render_descriptor.uniform_buffers_memory[i], 0, bufferSize, 0, &render_descriptor.uniform_buffers_mapped[i]);
    }
}

#endif
#define  DESCRIPTORSETS
