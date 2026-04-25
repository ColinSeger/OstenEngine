#pragma once
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include "../../../additional_things/arena.h"
#include "../../device/vulkan/device.h"
#include "../../texture/vulkan/texture.h"

/**
    The swap-chain is what is used to display images to the screen.

    The struct for the swap-chain is quite simple as it only needs to keep track of it's size format and vulkan's representation of a swap-chain
*/
typedef struct SwapChain{
    VkExtent2D screen_extent;

    VkSwapchainKHR swap_chain;

    VkFormat swap_chain_image_format;
} SwapChain;

/**
    The swap-chain-images is what stores the swap-chan images memory.

    size_t might seem confusing but is at the moment just a representation of where in a arena they are located and is used to access the arena memory.

    The VK attributes are what is used to communicate the data with the gpu
*/
typedef struct SwapChainImages{
    VkImage depth_image;

    VkDeviceMemory depth_image_memory;

    VkImageView depth_image_view;

    size_t swap_chain_images;//Could Probably just unify all images into one allocation

    size_t swap_chain_image_view;

    size_t swap_chain_frame_buffers;

    uint8_t image_amount;
} SwapChainImages;

/// @brief Clamps a uint32 value
static inline uint32_t simple_clamp(const uint32_t value, const  uint32_t min,const  uint32_t max){
    if(value > max){
        return max;
    }
    if(value < min){
        return min;
    }
    return value;
}

static inline VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR* surface_capabilities, VkExtent2D window) {
    if (surface_capabilities->currentExtent.width != UINT32_MAX) {
        return surface_capabilities->currentExtent;
    }

    window.width = simple_clamp(window.width, surface_capabilities->minImageExtent.width, surface_capabilities->maxImageExtent.width);
    window.height = simple_clamp(window.height, surface_capabilities->minImageExtent.height, surface_capabilities->maxImageExtent.height);

    return window;
}

static inline VkImageView create_depth_resources(Device* device, VkExtent2D image_size, VkDeviceMemory* depth_image_memory, VkImage* depth_image){
    VkFormat depth_formating = find_depth_formats(device->physical_device);

    create_image(device, image_size, depth_formating, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);

    return create_image_view(device->virtual_device, *depth_image, depth_formating, VK_IMAGE_ASPECT_DEPTH_BIT);
}

static VkSurfaceFormatKHR select_swap_surface_format(VkSurfaceFormatKHR* available_formats, uint32_t amount){
    for (uint32_t i = 0; i < amount ; i++) {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_formats[i];
        }
    }
    return available_formats[0];
}

static inline VkPresentModeKHR select_swap_present_mode(VkPresentModeKHR* available_present_modes, uint32_t amount) {
    for (uint32_t i = 0; i < amount ; i++) {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_modes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static inline VkResult create_swap_chain(Device* device, VkExtent2D window, VkSurfaceKHR surface, SwapChain* swap_chain, HeapStack* memory_arena){
    SwapChainSupportDetails swap_chain_support = find_swap_chain_support(device->physical_device, &surface, memory_arena);

    VkSurfaceFormatKHR surface_format = select_swap_surface_format((VkSurfaceFormatKHR*)swap_chain_support.surface_data, swap_chain_support.surface_amount);
    VkSurfaceFormatKHR* surface_end = (VkSurfaceFormatKHR*)swap_chain_support.surface_data;
    surface_end+= swap_chain_support.surface_amount;
    VkPresentModeKHR* offset = (VkPresentModeKHR*)surface_end;
    VkPresentModeKHR present_mode = select_swap_present_mode(offset, swap_chain_support.present_amount);

    VkExtent2D screen_extent = select_swap_chain_extent(&swap_chain_support.surface_capabilities, window);

    uint32_t image_amount = swap_chain_support.surface_capabilities.minImageCount + 1;

    if(swap_chain_support.surface_capabilities.maxImageCount > 0 && image_amount > swap_chain_support.surface_capabilities.maxImageCount){
        image_amount = swap_chain_support.surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_amount;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = screen_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = find_queue_families(device->physical_device, &surface, memory_arena);
    uint32_t queue_family_indices[] = { indices.graphics_family.number, indices.present_family.number};

    if (indices.graphics_family.number != indices.present_family.number) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = 0; // Optional
    }

    create_info.preTransform = swap_chain_support.surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device->virtual_device, &create_info, 0, &swap_chain->swap_chain);

    if(result != VK_SUCCESS){
        return result;
        //("Failed to create swap chain");
    }
    swap_chain->screen_extent = screen_extent;
    swap_chain->swap_chain_image_format = surface_format.format;
    return VK_SUCCESS;
}

static inline int clean_swap_chain(VkDevice virtual_device, SwapChain* swap_chain, SwapChainImages* swap_chain_images, HeapStack* heap_stack){
    vkDeviceWaitIdle(virtual_device);
    vkDestroyImageView(virtual_device, swap_chain_images->depth_image_view, 0);
    vkDestroyImage(virtual_device, swap_chain_images->depth_image, 0);
    vkFreeMemory(virtual_device, swap_chain_images->depth_image_memory, 0);

    for (int i = 0; i < swap_chain_images->image_amount; i++) {
        vkDestroyFramebuffer(virtual_device, ((VkFramebuffer*)get_at_index(heap_stack, swap_chain_images->swap_chain_frame_buffers))[i], 0);
        vkDestroyImageView(virtual_device, ((VkImageView*)get_at_index(heap_stack, swap_chain_images->swap_chain_image_view))[i], 0);
    }

    vkDestroySwapchainKHR(virtual_device, swap_chain->swap_chain, 0);
    vkDeviceWaitIdle(virtual_device);
    return 1;
}


static inline VkResult create_image_views(SwapChainImages* swap_images, VkDevice virtual_device, VkFormat image_format, HeapStack* heap_stack){
    swap_images->swap_chain_image_view = arena_alloc_memory(heap_stack, sizeof(VkImageView) * swap_images->image_amount);

    for (size_t i = 0; i < swap_images->image_amount; i++){
        VkImageViewCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = ((VkImage*)get_at_index(heap_stack, swap_images->swap_chain_images))[i];

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

        VkResult creation_status = vkCreateImageView(virtual_device, &create_info, 0, &((VkImageView*)get_at_index(heap_stack, swap_images->swap_chain_image_view))[i]);

        if(creation_status != VK_SUCCESS)
            //This Only happens if it failed to create image views
            return creation_status;
    }
    return VK_SUCCESS;
}

static inline VkResult create_swap_chain_images(Device* device, SwapChain* swap_chain, VkSurfaceKHR surface, SwapChainImages* swap_images, HeapStack* heap_stack){
    SwapChainSupportDetails swap_chain_support = find_swap_chain_support(device->physical_device, &surface, heap_stack);

    uint32_t image_amount = swap_chain_support.surface_capabilities.minImageCount + 1;

    swap_images->swap_chain_images = arena_alloc_memory(heap_stack, sizeof(VkImage) * image_amount);

    swap_images->image_amount = image_amount;
    //swap_images.swap_chain_images = images;//Should Probably allocate for this

    vkGetSwapchainImagesKHR(device->virtual_device, swap_chain->swap_chain, &image_amount, (VkImage*)get_at_index(heap_stack, swap_images->swap_chain_images));

    VkResult images_result = create_image_views(swap_images, device->virtual_device, swap_chain->swap_chain_image_format, heap_stack);
    return images_result;
}

static inline VkResult create_frame_buffers(SwapChainImages* swap_images, VkDevice virtual_device, VkRenderPass render_pass, VkImageView depth_image_view, VkExtent2D extent, HeapStack* heap_stack){
    swap_images->swap_chain_frame_buffers = arena_alloc_memory(heap_stack, sizeof(VkFramebuffer) * swap_images->image_amount);

    for (size_t i = 0; i < swap_images->image_amount; i++) {
        VkImageView attachments[2] = {
            ((VkImageView*)get_at_index(heap_stack, swap_images->swap_chain_image_view))[i],
            depth_image_view
        };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = extent.width;
        framebuffer_info.height = extent.height;
        framebuffer_info.layers = 1;
        VkResult creation_status = vkCreateFramebuffer(virtual_device, &framebuffer_info, 0, &((VkFramebuffer*)get_at_index(heap_stack, swap_images->swap_chain_frame_buffers))[i]);
        if(creation_status != VK_SUCCESS)
            //This Only happens if it failed to create framebuffers
            return creation_status;
    }
    return VK_SUCCESS;
}

static inline void start_render_pass(VkCommandBuffer command_buffer, VkFramebuffer frame_buffer, VkRenderPass render_pass, VkExtent2D viewport_extent){
    VkClearValue clear_values[2] = {};
    VkClearColorValue color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    VkClearDepthStencilValue depth_stencil = {1.0f, 0};
    clear_values[0].color = color;
    clear_values[1].depthStencil = depth_stencil;

    //Begining of render pass
    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = frame_buffer;
    render_pass_info.renderArea.offset = (VkOffset2D){0, 0};
    render_pass_info.renderArea.extent = viewport_extent;
    render_pass_info.clearValueCount = sizeof(clear_values) / sizeof(clear_values[0]);
    render_pass_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}
inline VkResult end_render_pass(VkCommandBuffer command_buffer){
    return vkEndCommandBuffer(command_buffer);
}

static inline void bind_pipeline(VkCommandBuffer command_buffer, VkPipeline pipeline, VkExtent2D extent){
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}
