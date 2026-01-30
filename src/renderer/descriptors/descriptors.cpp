#include <cstddef>
#ifndef DESCRIPTORSETS
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../../../external/math_3d.h"
#include "../device/vulkan/device.cpp"

constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;

typedef struct{
    uint32_t object_amount;
    VkBuffer uniform_buffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniform_buffers_memory[MAX_FRAMES_IN_FLIGHT];
    void* uniform_buffers_mapped[MAX_FRAMES_IN_FLIGHT];
} RenderDescriptors;

typedef struct{
    VkDescriptorSet light_descriptor_sets[MAX_FRAMES_IN_FLIGHT];

    VkBuffer uniform_buffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniform_buffers_memory[MAX_FRAMES_IN_FLIGHT];
    void* uniform_buffers_mapped[MAX_FRAMES_IN_FLIGHT];
} Light;

typedef struct{
    VkBuffer uniform_buffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniform_buffers_memory[MAX_FRAMES_IN_FLIGHT];
    void* uniform_buffers_mapped[MAX_FRAMES_IN_FLIGHT];
} CameraDescriptor;

typedef struct{
    VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT];
} RenderingDescriptor;

typedef struct{
    VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT];
} TextureDescriptor;

typedef struct{
    //mat4_t model;
    mat4_t view;
    mat4_t projection;
} CameraUbo;

typedef struct{
    //mat4_t model;
    mat4_t view;
    mat4_t projection;
} LightUbo;

typedef struct{
    mat4_t model;
} ObjectUBO;

void create_descriptor_pool(VkDescriptorPool& result, VkDevice virtual_device, const uint32_t pool_size){
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         MAX_FRAMES_IN_FLIGHT * pool_size},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * pool_size},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         MAX_FRAMES_IN_FLIGHT * pool_size},
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

VkResult create_forward_descriptor_set_layout(VkDevice virtual_device, VkDescriptorSetLayout* descriptor_set_layout){
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding model_layout_binding{};
    model_layout_binding.binding = 1;
    model_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    model_layout_binding.descriptorCount = 1;
    model_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    model_layout_binding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding bindings[] = { ubo_layout_binding, model_layout_binding};
    constexpr size_t bindings_amount = sizeof(bindings) / sizeof(bindings[0]);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings_amount;
    layoutInfo.pBindings = bindings;

    return vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, descriptor_set_layout);
}

VkResult create_fragment_layout(VkDevice virtual_device, VkDescriptorSetLayout* descriptor_set_layout)
{
    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 0;
    sampler_layout_binding.descriptorCount = 2;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { sampler_layout_binding };
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

    VkDescriptorSetLayoutBinding model_layout_binding{};
    model_layout_binding.binding = 1;
    model_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    model_layout_binding.descriptorCount = 1;
    model_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { ubo_layout_binding , model_layout_binding};
    constexpr size_t bindings_amount = sizeof(bindings) / sizeof(bindings[0]);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings_amount;
    layoutInfo.pBindings = bindings;

    return vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, descriptor_set_layout);
}

VkResult create_shadow_sets(VkDevice virtual_device, Light& lighy, RenderDescriptors render_buffer, RenderingDescriptor& render_data, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout){
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo camera_info{};
        camera_info.offset = 0;
        camera_info.range = sizeof(CameraUbo);
        camera_info.buffer = lighy.uniform_buffers[i];

        VkDescriptorBufferInfo buffer_info{};
        buffer_info.offset = 0;
        buffer_info.range = sizeof(ObjectUBO) * render_buffer.object_amount;
        buffer_info.buffer = render_buffer.uniform_buffers[i];

        constexpr uint32_t descriptor_size = 2;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = lighy.light_descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &camera_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = lighy.light_descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }

    return VK_SUCCESS;
}

void create_fragment_set(VkDevice virtual_device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, TextureDescriptor& descriptor, VkImageView image_view, VkSampler sampler){

    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts;

    VkResult allocation_status = vkAllocateDescriptorSets(virtual_device, &allocInfo, descriptor.descriptor_sets);

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    VkDescriptorImageInfo image_descriptors_info[] = {image_info};
    constexpr uint32_t image_amount = sizeof(image_descriptors_info) / sizeof(image_descriptors_info[0]);


    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        constexpr uint32_t descriptor_size = 1;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor.descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[0].descriptorCount = image_amount;
        descriptor_writes[0].pImageInfo = image_descriptors_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
}

void create_descriptor_set(VkDevice virtual_device, RenderingDescriptor& rendering_descriptor, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, CameraDescriptor& camera, RenderDescriptors& objects) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts;

    VkResult allocation_status = vkAllocateDescriptorSets(virtual_device, &allocInfo, rendering_descriptor.descriptor_sets);

    if(allocation_status != VK_SUCCESS)
        throw("Failed to create descriptor sets");


    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo camera_info{};
        camera_info.offset = 0;
        camera_info.range = sizeof(CameraUbo);
        camera_info.buffer = camera.uniform_buffers[i];

        VkDescriptorBufferInfo buffer_info{};
        buffer_info.offset = 0;
        buffer_info.range = sizeof(ObjectUBO) * objects.object_amount;
        buffer_info.buffer = objects.uniform_buffers[i];

        constexpr uint32_t descriptor_size = 2;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = rendering_descriptor.descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &camera_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = rendering_descriptor.descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
}

void update_descriptor_set(VkDevice virtual_device, RenderingDescriptor& rendering_descriptor, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, CameraDescriptor& camera, RenderDescriptors& objects) {

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo camera_info{};
        camera_info.offset = 0;
        camera_info.range = sizeof(CameraUbo);
        camera_info.buffer = camera.uniform_buffers[i];

        VkDescriptorBufferInfo buffer_info{};
        buffer_info.offset = 0;
        buffer_info.range = sizeof(ObjectUBO) * objects.object_amount;
        buffer_info.buffer = objects.uniform_buffers[i];

        constexpr uint32_t descriptor_size = 2;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = rendering_descriptor.descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &camera_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = rendering_descriptor.descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
}

void create_uniform_buffers(RenderDescriptors* render_descriptors, uint32_t amount, Device device) {
    VkDeviceSize bufferSize = sizeof(ObjectUBO) * amount;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CommandBuffer::create_buffer(
            device,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            bufferSize,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            render_descriptors->uniform_buffers[i],
            render_descriptors->uniform_buffers_memory[i]
        );

        vkMapMemory(device.virtual_device, render_descriptors->uniform_buffers_memory[i], 0, bufferSize, 0, &render_descriptors->uniform_buffers_mapped[i]);
    }
}

void create_uniform_buffer(RenderDescriptors& render_descriptor, Device& device) {
    constexpr VkDeviceSize bufferSize = sizeof(ObjectUBO);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CommandBuffer::create_buffer(
            device,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            bufferSize,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            render_descriptor.uniform_buffers[i],
            render_descriptor.uniform_buffers_memory[i]
        );

        VkResult result = vkMapMemory(device.virtual_device, render_descriptor.uniform_buffers_memory[i], 0, bufferSize, 0, &render_descriptor.uniform_buffers_mapped[i]);
    }
}

void create_light_uniform_buffer(Light* light_descriptor, Device& device) {
    VkDeviceSize bufferSize = sizeof(LightUbo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CommandBuffer::create_buffer(
            device,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            bufferSize,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            light_descriptor->uniform_buffers[i],
            light_descriptor->uniform_buffers_memory[i]
        );

        vkMapMemory(device.virtual_device, light_descriptor->uniform_buffers_memory[i], 0, bufferSize, 0, &light_descriptor->uniform_buffers_mapped[i]);
    }
}

void create_camera_uniform_buffer(CameraDescriptor& render_descriptor, Device& device) {
    VkDeviceSize bufferSize = sizeof(CameraUbo);

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
