#include <cstddef>
#include <cstring>
#ifndef DESCRIPTORSETS
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include "../../../external/math_3d.h"
#include "../device/vulkan/device.cpp"
#include "../texture/vulkan/texture.cpp"

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

struct RenderAble{
    VkDescriptorSet model_descriptor_sets[MAX_FRAMES_IN_FLIGHT];
    uint32_t capacity;
    uint32_t instance_amount;
    uint32_t texture_index;
    uint16_t model_index;
    uint16_t transform_index;
};

struct ModelData{
    VkBuffer uniform_buffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniform_buffers_memory[MAX_FRAMES_IN_FLIGHT];
    void* uniform_buffers_mapped[MAX_FRAMES_IN_FLIGHT];
    size_t renderable_memory_index;
    uint32_t object_capacity;
    uint16_t renderable_amount;
    // uint16_t renderable_capacity;
};

typedef struct{
    VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT];
} RenderingDescriptor;

typedef struct{
    VkDescriptorSet descriptor_sets[MAX_FRAMES_IN_FLIGHT];
} TextureDescriptor;

typedef struct{
    mat4_t view;
    mat4_t projection;
} CameraUbo;

typedef struct{
    mat4_t view;
    mat4_t projection;
} LightUbo;

typedef struct{
    mat4_t model;
} ObjectUBO;

constexpr uint32_t texture_capacity = 20;
VkDescriptorImageInfo image_descriptors_info[texture_capacity] = {};


//Can Return null if you are accesing outside capacity
static RenderAble* get_renderable(ModelData& model_data, uint32_t index, HeapStack heap_stack){
    if(index > model_data.renderable_amount) return 0;

    RenderAble* render_data = (RenderAble*)heap_stack[model_data.renderable_memory_index];

    return &render_data[index];
}

//Can return null if you are out of capacity
static RenderAble* get_free_renderable(ModelData& model_data, HeapStack heap_stack, uint32_t* index){
    RenderAble* render_data = (RenderAble*)heap_stack[model_data.renderable_memory_index];
    for (uint32_t i = 0; i < model_data.object_capacity; i++) {
        if(render_data->capacity <= 0) return render_data;
        render_data++;
        *index += 1;
    }
    return 0;
}

static ObjectUBO* get_mapped_uniforms(ModelData& model_data, HeapStack heap_stack, int32_t render_index, uint8_t frame){
    if(render_index > model_data.renderable_amount) return nullptr;
    ObjectUBO* result = (ObjectUBO*)model_data.uniform_buffers_mapped[frame];

    RenderAble* render_data = (RenderAble*)heap_stack[model_data.renderable_memory_index];

    for (int i = 0; i < render_index; i++) {
        result += render_data[i].capacity;
    }
    return result;
}

static size_t get_required_offset(ModelData& model_data, HeapStack heap_stack, int32_t render_index){
    if(render_index > model_data.renderable_amount) return 0;
    size_t result = 0;

    RenderAble* render_data = (RenderAble*)heap_stack[model_data.renderable_memory_index];

    for (int i = 0; i < render_index; i++) {
        result += render_data[i].capacity * sizeof(ObjectUBO);
    }
    return result;
}

static VkResult create_descriptor_pool(VkDescriptorPool& result, VkDevice virtual_device, const uint32_t pool_size){
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

    return vkCreateDescriptorPool(virtual_device, &pool_info, nullptr, &result);
}

static VkResult create_forward_descriptor_set_layout(VkDevice virtual_device, VkDescriptorSetLayout* descriptor_set_layout){
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding ubo_light_layout_binding{};
    ubo_light_layout_binding.binding = 1;
    ubo_light_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_light_layout_binding.descriptorCount = 1;
    ubo_light_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { ubo_layout_binding, ubo_light_layout_binding };
    constexpr size_t bindings_amount = sizeof(bindings) / sizeof(bindings[0]);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings_amount;
    layoutInfo.pBindings = bindings;

    return vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, descriptor_set_layout);
}

static VkResult create_model_set_layout(VkDevice virtual_device, VkDescriptorSetLayout* descriptor_set_layout){

    VkDescriptorSetLayoutBinding model_layout_binding{};
    model_layout_binding.binding = 0;
    model_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    model_layout_binding.descriptorCount = 1;
    model_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { model_layout_binding };
    constexpr uint32_t bindings_amount = sizeof(bindings) / sizeof(bindings[0]);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings_amount;
    layoutInfo.pBindings = bindings;

    return vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, descriptor_set_layout);
}

static VkResult create_fragment_layout(VkDevice virtual_device, VkDescriptorSetLayout* descriptor_set_layout){
    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 0;
    sampler_layout_binding.descriptorCount = texture_capacity;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding shadow_sampler_layout_binding{};
    shadow_sampler_layout_binding.binding = 1;
    shadow_sampler_layout_binding.descriptorCount = 1;
    shadow_sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadow_sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 2;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { sampler_layout_binding, shadow_sampler_layout_binding, ubo_layout_binding };
    constexpr size_t bindings_amount = sizeof(bindings) / sizeof(bindings[0]);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings_amount;
    layoutInfo.pBindings = bindings;

    return vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, descriptor_set_layout);
}

static VkResult create_shadow_set_layout(VkDevice virtual_device, VkDescriptorSetLayout* descriptor_set_layout){
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

static VkResult create_shadow_sets(VkDevice virtual_device, CameraDescriptor light, RenderingDescriptor& render_data, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout){
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    VkResult allocation_status = vkAllocateDescriptorSets(virtual_device, &allocInfo, render_data.descriptor_sets);

    if(allocation_status != VK_SUCCESS)
        return allocation_status;


    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo camera_info{};
        camera_info.offset = 0;
        camera_info.range = sizeof(CameraUbo);
        camera_info.buffer = light.uniform_buffers[i];

        constexpr uint32_t descriptor_size = 1;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = render_data.descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &camera_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
    return VK_SUCCESS;
}


static void create_fragment_set(VkDevice virtual_device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, TextureDescriptor& descriptor, VkImageView image_view, VkSampler sampler, VkBuffer lisght, TextureImage texture){

    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    VkResult allocation_status = vkAllocateDescriptorSets(virtual_device, &allocInfo, descriptor.descriptor_sets);

    if(allocation_status != VK_SUCCESS) return;

    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    for(uint32_t i = 0; i < texture_capacity; i++){
        VkDescriptorImageInfo texture_info{
            .sampler = texture.texture_sampler,
            .imageView = texture.image_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        image_descriptors_info[i] = texture_info;
    }
    //constexpr uint32_t image_amount = sizeof(image_descriptors_info) / sizeof(image_descriptors_info[0]);

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo camera_info{};
        camera_info.offset = 0;
        camera_info.range = sizeof(vec3_t);
        camera_info.buffer = lisght;

        constexpr uint32_t descriptor_size = 3;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor.descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[0].descriptorCount = texture_capacity;
        descriptor_writes[0].pImageInfo = image_descriptors_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptor.descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[2].dstSet = descriptor.descriptor_sets[i];
        descriptor_writes[2].dstBinding = 2;
        descriptor_writes[2].dstArrayElement = 0;
        descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[2].descriptorCount = 1;
        descriptor_writes[2].pBufferInfo = &camera_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
}

static void create_fragment_set2(VkDevice virtual_device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, TextureDescriptor& descriptor, VkImageView image_view, VkSampler sampler, VkBuffer lisght, uint16_t texture_index){
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = image_view;
    image_info.sampler = sampler;

    //Not good
    image_descriptors_info[texture_index].imageView = loaded_textures[texture_index].image_view;
    image_descriptors_info[texture_index].sampler = loaded_textures[texture_index].texture_sampler;

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo camera_info{};
        camera_info.offset = 0;
        camera_info.range = sizeof(vec3_t);
        camera_info.buffer = lisght;

        constexpr uint32_t descriptor_size = 3;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor.descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[0].descriptorCount = texture_capacity;
        descriptor_writes[0].pImageInfo = image_descriptors_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptor.descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[2].dstSet = descriptor.descriptor_sets[i];
        descriptor_writes[2].dstBinding = 2;
        descriptor_writes[2].dstArrayElement = 0;
        descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[2].descriptorCount = 1;
        descriptor_writes[2].pBufferInfo = &camera_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
}

// static void update_fragment_set(VkDevice virtual_device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, TextureDescriptor& descriptor, VkImageView image_view, VkSampler sampler, VkBuffer lisght, TextureImage texture){
//     VkDescriptorImageInfo image_info{};
//     image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//     image_info.imageView = image_view;
//     image_info.sampler = sampler;

//     VkDescriptorImageInfo texture_info{};
//     texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//     texture_info.imageView = texture.image_view;
//     texture_info.sampler = texture.texture_sampler;

//     //VkDescriptorImageInfo image_descriptors_info[] = {image_info, texture_info};
//     //constexpr uint32_t image_amount = sizeof(image_descriptors_info) / sizeof(image_descriptors_info[0]);

//     for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//         VkDescriptorBufferInfo camera_info{};
//         camera_info.offset = 0;
//         camera_info.range = sizeof(vec3_t);
//         camera_info.buffer = lisght;

//         constexpr uint32_t descriptor_size = 3;
//         VkWriteDescriptorSet descriptor_writes[descriptor_size]{};

//         descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptor_writes[0].dstSet = descriptor.descriptor_sets[i];
//         descriptor_writes[0].dstBinding = 0;
//         descriptor_writes[0].dstArrayElement = 0;
//         descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptor_writes[0].descriptorCount = 1;
//         descriptor_writes[0].pImageInfo = &texture_info;

//         descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptor_writes[1].dstSet = descriptor.descriptor_sets[i];
//         descriptor_writes[1].dstBinding = 1;
//         descriptor_writes[1].dstArrayElement = 0;
//         descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptor_writes[1].descriptorCount = 1;
//         descriptor_writes[1].pImageInfo = &image_info;

//         descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptor_writes[2].dstSet = descriptor.descriptor_sets[i];
//         descriptor_writes[2].dstBinding = 2;
//         descriptor_writes[2].dstArrayElement = 0;
//         descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//         descriptor_writes[2].descriptorCount = 1;
//         descriptor_writes[2].pBufferInfo = &camera_info;

//         vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
//     }
// }

static VkResult create_descriptor_set(VkDevice virtual_device, RenderingDescriptor& rendering_descriptor, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, CameraDescriptor& camera, CameraDescriptor& light) {
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    VkResult allocation_status = vkAllocateDescriptorSets(virtual_device, &allocInfo, rendering_descriptor.descriptor_sets);

    if(allocation_status != VK_SUCCESS)
        return allocation_status;


    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo camera_info{};
        camera_info.offset = 0;
        camera_info.range = sizeof(CameraUbo);
        camera_info.buffer = camera.uniform_buffers[i];

        VkDescriptorBufferInfo light_info{};
        light_info.offset = 0;
        light_info.range = sizeof(CameraUbo);
        light_info.buffer = light.uniform_buffers[i];

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
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = &light_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }
    return VK_SUCCESS;
}

static VkResult create_model_set(VkDevice virtual_device, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout, ModelData& model_data, uint32_t render_able_index, HeapStack heap_stack){
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {};
    for(uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        layouts[i] = descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    RenderAble* render_able = (RenderAble*)get_renderable(model_data, render_able_index, heap_stack);
    VkResult allocation_status = vkAllocateDescriptorSets(virtual_device, &allocInfo, render_able->model_descriptor_sets);
    if(allocation_status) return allocation_status;

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.offset  = get_required_offset(model_data, heap_stack, render_able_index);
        buffer_info.range   = sizeof(ObjectUBO) * render_able->capacity;
        buffer_info.buffer  = model_data.uniform_buffers[i];

        constexpr uint32_t descriptor_size = 1;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = render_able->model_descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
    }

    return VK_SUCCESS;
}

static void create_light_uniform_buffer(Light* light_descriptor, Device& device) {
    VkDeviceSize bufferSize = sizeof(LightUbo);

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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

static void create_camera_uniform_buffer(CameraDescriptor& render_descriptor, Device& device) {
    VkDeviceSize bufferSize = sizeof(CameraUbo);

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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

static VkResult init_model_data(ModelData& model_data, Device& device, HeapStack& heap_stack){
    VkDeviceSize bufferSize = sizeof(ObjectUBO) * model_data.object_capacity;
    model_data.renderable_amount = 0;

    model_data.renderable_memory_index = arena_alloc_memory(heap_stack, 50 * sizeof(RenderAble));

    //Temp
    memset(heap_stack[model_data.renderable_memory_index], 0, 50 * sizeof(RenderAble));
    //EndTemp

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkResult result = CommandBuffer::create_buffer(
            device,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            bufferSize,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            model_data.uniform_buffers[i],
            model_data.uniform_buffers_memory[i]
        );

        if(result != VK_SUCCESS) return result;

        vkMapMemory(device.virtual_device, model_data.uniform_buffers_memory[i], 0, bufferSize, 0, &model_data.uniform_buffers_mapped[i]);
    }

    return VK_SUCCESS;
}

#endif
#define  DESCRIPTORSETS
