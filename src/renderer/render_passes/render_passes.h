#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include "../device/vulkan/device.cpp"
#include "../texture/vulkan/texture.cpp"
#include "vulkan/vulkan_core.h"

static inline VkResult create_render_pass(VkRenderPass* render_pass, VkFormat swap_chain_format, const Device& device){
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription color_attachment{};
    color_attachment.format = swap_chain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = Texture::find_depth_formats(device.physical_device);
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription description_attachments[2] = {color_attachment, depth_attachment};
    uint32_t description_amount = sizeof(description_attachments) / sizeof(description_attachments[0]);

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub_pass{};
    sub_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub_pass.colorAttachmentCount = 1;
    sub_pass.pColorAttachments = &color_attachment_ref;
    sub_pass.pDepthStencilAttachment = &depth_attachment_ref;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = description_amount;
    render_pass_info.pAttachments = description_attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &sub_pass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    return vkCreateRenderPass(device.virtual_device, &render_pass_info, nullptr, render_pass);
}

static inline VkResult create_offscreen_render_pass(VkRenderPass* render_pass, const Device& device){
    VkSubpassDependency dependencies[2]{};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Shadow pass → external (for sampling)
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = VK_FORMAT_D32_SFLOAT;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 0;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub_pass{};
    sub_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub_pass.colorAttachmentCount = 0;
    sub_pass.pDepthStencilAttachment = &depth_attachment_ref;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &depth_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &sub_pass;
    render_pass_info.dependencyCount = 2;
    render_pass_info.pDependencies = dependencies;

    return vkCreateRenderPass(device.virtual_device, &render_pass_info, nullptr, render_pass);
}

struct ShadowPass{
    VkImage depth_image;
    VkDeviceMemory depth_image_memory;
    VkImageView image_view;
    VkSampler sampler;
    VkRenderPass render_pass;
    VkFramebuffer framebuffer;
    VkSampler debug_sampler;
};

//Temporaraly placed here
static inline void create_offscreen_framebuffer(const Device device, const VkExtent2D size, ShadowPass* shadow_pass){

    Texture::create_image(
        device,
        size,
        VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        shadow_pass->depth_image,
        shadow_pass->depth_image_memory
    );

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = shadow_pass->depth_image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = VK_FORMAT_D32_SFLOAT;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VkResult creation_status = vkCreateImageView(device.virtual_device, &image_view_create_info, nullptr, &shadow_pass->image_view);

    VkSamplerCreateInfo sampler_create_info{};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.compareEnable = VK_TRUE;
    sampler_create_info.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.mipLodBias = 0.f;
    sampler_create_info.maxAnisotropy = 1.f;
    sampler_create_info.minLod = 0.f;
    sampler_create_info.maxLod = 1.f;
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    VkSamplerCreateInfo debug_sampler{};
    debug_sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    debug_sampler.compareEnable = VK_FALSE;
    debug_sampler.minFilter = VK_FILTER_LINEAR;
    debug_sampler.magFilter = VK_FILTER_LINEAR;
    debug_sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    debug_sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    debug_sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    debug_sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    debug_sampler.minLod = 0.0f;
    debug_sampler.maxLod = 1.0f;
    debug_sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;


    creation_status = vkCreateSampler(device.virtual_device, &sampler_create_info, nullptr, &shadow_pass->sampler);
    creation_status = vkCreateSampler(device.virtual_device, &debug_sampler, nullptr, &shadow_pass->debug_sampler);

    create_offscreen_render_pass(&shadow_pass->render_pass, device);

    VkFramebufferCreateInfo frame_buffer_create_info{};
    frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_create_info.renderPass = shadow_pass->render_pass;
    frame_buffer_create_info.attachmentCount = 1;
    frame_buffer_create_info.pAttachments = &shadow_pass->image_view;
    frame_buffer_create_info.width = size.width;
    frame_buffer_create_info.height = size.height;
    frame_buffer_create_info.layers = 1;
    frame_buffer_create_info.flags = 0;

    vkCreateFramebuffer(device.virtual_device, &frame_buffer_create_info, nullptr, &shadow_pass->framebuffer);
}
