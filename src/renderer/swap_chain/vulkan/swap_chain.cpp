#pragma once
#include <cstddef>
#include <vulkan/vulkan_core.h>
#include <cstdlib>
#include "../../../additional_things/arena.h"
#include "../../device/vulkan/device.cpp"
#include "../../texture/vulkan/texture.cpp"

typedef struct
{
    VkBuffer& vertex_buffer;
    VkBuffer& index_buffer;
} RenderBuffer;

typedef struct
{
    VkExtent2D screen_extent;

    VkSwapchainKHR swap_chain;

    VkFormat swap_chain_image_format;
} SwapChain;

typedef struct
{
    VkImage depth_image;

    VkDeviceMemory depth_image_memory;

    VkImageView depth_image_view;

    size_t swap_chain_images;//Could Probably just unify all images into one allocation

    size_t swap_chain_image_view;

    size_t swap_chain_frame_buffers;

    uint8_t image_amount;
} SwapChainImages;

static constexpr uint32_t simple_clamp(const uint32_t value, const  uint32_t min,const  uint32_t max)
{
    if(value > max){
        return max;
    }
    if(value < min){
        return min;
    }
    return value;
}

static VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities, VkExtent2D window) {

    if (surface_capabilities.currentExtent.width != UINT32_MAX) {
        return surface_capabilities.currentExtent;
    }

    window.width = simple_clamp(window.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
    window.height = simple_clamp(window.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

     return window;
}

VkImageView create_depth_resources(Device& device, VkExtent2D image_size, VkDeviceMemory& depth_image_memory, VkImage& depth_image)
{
    VkFormat depth_formating = Texture::find_depth_formats(device.physical_device);

    Texture::create_image(device, image_size, depth_formating, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);

    return Texture::create_image_view(device.virtual_device ,depth_image, depth_formating, VK_IMAGE_ASPECT_DEPTH_BIT);
}

static VkSurfaceFormatKHR select_swap_surface_format(const VkSurfaceFormatKHR* available_formats, const uint32_t amount){
    for (uint32_t i = 0; i < amount ; i++) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }
    return available_formats[0];
}

static VkPresentModeKHR select_swap_present_mode(const VkPresentModeKHR* available_present_modes, const uint32_t amount) {
    for (uint32_t i = 0; i < amount ; i++) {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

void create_swap_chain(Device& device, const VkExtent2D window, VkSurfaceKHR surface, SwapChain& swap_chain, MemArena& memory_arena){
    SwapChainSupportDetails swap_chain_support = find_swap_chain_support(device.physical_device, surface, memory_arena);

    VkSurfaceFormatKHR surface_format = select_swap_surface_format((VkSurfaceFormatKHR*)swap_chain_support.surface_data, swap_chain_support.surface_amount);
    VkSurfaceFormatKHR* surface_end = (VkSurfaceFormatKHR*)swap_chain_support.surface_data;
    surface_end+= swap_chain_support.surface_amount;
    VkPresentModeKHR* offset = (VkPresentModeKHR*)surface_end;
    VkPresentModeKHR present_mode = select_swap_present_mode(offset, swap_chain_support.present_amount);

    VkExtent2D screen_extent = select_swap_chain_extent(swap_chain_support.surface_capabilities, window);

    uint32_t image_amount = swap_chain_support.surface_capabilities.minImageCount + 1;

    if(swap_chain_support.surface_capabilities.maxImageCount > 0 && image_amount > swap_chain_support.surface_capabilities.maxImageCount){
        image_amount = swap_chain_support.surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_amount;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = screen_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndicies indices = find_queue_families(device.physical_device, surface, memory_arena);
    uint32_t queue_family_indices[] = { indices.graphics_family.number, indices.present_family.number};

    if (indices.graphics_family.number != indices.present_family.number) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    create_info.preTransform = swap_chain_support.surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device.virtual_device, &create_info, nullptr, &swap_chain.swap_chain);

    if(result != VK_SUCCESS){
        throw("Failed to create swap chain");
    }
    swap_chain.screen_extent = screen_extent;
    swap_chain.swap_chain_image_format = surface_format.format;
}

int clean_swap_chain(VkDevice& virtual_device, SwapChain& swap_chain, SwapChainImages& swap_chain_images, MemArena& memory_arena)
{
    vkDeviceWaitIdle(virtual_device);
    vkDestroyImageView(virtual_device, swap_chain_images.depth_image_view, nullptr);
    vkDestroyImage(virtual_device, swap_chain_images.depth_image, nullptr);
    vkFreeMemory(virtual_device, swap_chain_images.depth_image_memory, nullptr);

    for (int i = 0; i < swap_chain_images.image_amount; i++) {
        vkDestroyFramebuffer(virtual_device, ((VkFramebuffer*)memory_arena[swap_chain_images.swap_chain_frame_buffers])[i], nullptr);
        vkDestroyImageView(virtual_device, ((VkImageView*)memory_arena[swap_chain_images.swap_chain_image_view])[i], nullptr);
    }

    vkDestroySwapchainKHR(virtual_device, swap_chain.swap_chain, nullptr);
    vkDeviceWaitIdle(virtual_device);
    return 1;
}


static VkResult create_image_views(SwapChainImages& swap_images, VkDevice virtual_device, VkFormat image_format, MemArena& memory_arena){
    swap_images.swap_chain_image_view = arena_alloc_memory(memory_arena, sizeof(VkImageView) * swap_images.image_amount);

    for (size_t i = 0; i < swap_images.image_amount; i++)
    {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = static_cast<VkImage*>(memory_arena[swap_images.swap_chain_images])[i];

        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = image_format;

        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkResult creation_status = vkCreateImageView(virtual_device, &create_info, nullptr, &static_cast<VkImageView*>(memory_arena[swap_images.swap_chain_image_view])[i]);

        if(creation_status != VK_SUCCESS)
            //This Only happens if it failed to create image views
            return creation_status;
    }
    return VK_SUCCESS;
}

void create_swap_chain_images(Device& device, SwapChain& swap_chain,  VkSurfaceKHR surface, SwapChainImages& swap_images, MemArena& memory_arena)
{
    SwapChainSupportDetails swap_chain_support = find_swap_chain_support(device.physical_device, surface, memory_arena);

    uint32_t image_amount = swap_chain_support.surface_capabilities.minImageCount + 1;

    swap_images.swap_chain_images = arena_alloc_memory(memory_arena, sizeof(VkImage) * image_amount);

    swap_images.image_amount = image_amount;
    //swap_images.swap_chain_images = images;//Should Probably allocate for this

    vkGetSwapchainImagesKHR(device.virtual_device, swap_chain.swap_chain, &image_amount, (VkImage*)memory_arena[swap_images.swap_chain_images]);

    VkResult images_result = create_image_views(swap_images, device.virtual_device, swap_chain.swap_chain_image_format, memory_arena);

    if(images_result != VK_SUCCESS) throw "Failed to create swapchain images";
}

VkResult create_frame_buffers(SwapChainImages& swap_images, VkDevice virtual_device, VkRenderPass& render_pass, VkImageView depth_image_view, VkExtent2D extent, MemArena& memory_arena)
{
    swap_images.swap_chain_frame_buffers = arena_alloc_memory(memory_arena, sizeof(VkFramebuffer) * swap_images.image_amount);

    for (size_t i = 0; i < swap_images.image_amount; i++) {
        VkImageView attachments[2] = {
            static_cast<VkImageView*>(memory_arena[swap_images.swap_chain_image_view])[i],
            depth_image_view
        };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = extent.width;
        framebuffer_info.height = extent.height;
        framebuffer_info.layers = 1;
        VkResult creation_status = vkCreateFramebuffer(virtual_device, &framebuffer_info, nullptr, &static_cast<VkFramebuffer*>(memory_arena[swap_images.swap_chain_frame_buffers])[i]);
        if(creation_status != VK_SUCCESS)
            //This Only happens if it failed to create framebuffers
            return creation_status;
    }
    return VK_SUCCESS;
}

void start_render_pass(VkCommandBuffer& command_buffer, VkFramebuffer& frame_buffer, VkRenderPass render_pass, VkExtent2D viewport_extent)
{
    //Begining of render pass
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = frame_buffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = viewport_extent;

    VkClearValue clear_values[2]{};
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = sizeof(clear_values) / sizeof(clear_values[0]);
    render_pass_info.pClearValues = clear_values;
    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}
inline VkResult end_render_pass(VkCommandBuffer& command_buffer)
{
    return vkEndCommandBuffer(command_buffer);
}

void bind_pipeline(VkCommandBuffer& command_buffer, VkPipeline pipeline, VkExtent2D extent)
{
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

struct OffScreenImage{
    VkImage depth_image;

    VkDeviceMemory depth_image_memory;

    VkImageView depth_image_view;

    VkImage swap_chain_images;
    VkDeviceMemory image_memory;

    VkImageView swap_chain_image_view;

    VkFramebuffer* swap_chain_frame_buffers;
};

OffScreenImage create_offscreen_image(Device& device, VkExtent2D extent, VkRenderPass render_pass, VkImageView depth_image)
{
    OffScreenImage offscreen_image;
    offscreen_image.swap_chain_frame_buffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer));

    Texture::create_image(
        device,
        extent,
        VK_FORMAT_B8G8R8A8_SRGB ,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        offscreen_image.swap_chain_images,
        offscreen_image.image_memory
    );

    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = offscreen_image.swap_chain_images;

    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = VK_FORMAT_B8G8R8A8_SRGB ;

    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkResult creation_status = vkCreateImageView(device.virtual_device, &create_info, nullptr, &offscreen_image.swap_chain_image_view);

    VkImageView attachments[2] = {
        offscreen_image.swap_chain_image_view,
        depth_image
    };

    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass;
    framebuffer_info.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = extent.width;
    framebuffer_info.height = extent.height;
    framebuffer_info.layers = 1;

    creation_status = vkCreateFramebuffer(device.virtual_device, &framebuffer_info, nullptr, offscreen_image.swap_chain_frame_buffers);

    return offscreen_image;
}
