// #pragma once
// #include "../../common_includes.h"
// #include "../renderable.h"
// #include <vulkan/vulkan_core.h>

// static uint8_t FRAMES = 2;

// struct UniformBufferObject {
//     mat4_t model;
//     mat4_t view;
//     mat4_t proj;
// };

// void create_descriptor_pool(VkDescriptorPool& result, VkDevice virtual_device);

// void create_descriptor_set_layout(VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout);

// void create_descriptor_set(VkDevice virtual_device, Renderable& render_this, VkDescriptorPool& descriptor_pool, VkDescriptorSetLayout& descriptor_set_layout, VkImageView image_view, VkSampler sampler);

// void create_descriptor_sets(VkDescriptorPool& descriptor_pool, VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout, VkImageView image_view, VkSampler sampler, std::vector<Renderable>& to_render);
