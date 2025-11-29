#include "swap_chain.h"

uint32_t simple_clamp(uint32_t value, uint32_t min, uint32_t max)
{
    if(value > max){
        return max;
    }
    if(value < min){
        return min;
    }
    return value;
}

VkExtent2D select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities, GLFWwindow* window) {
    
    if (surface_capabilities.currentExtent.width != UINT32_MAX) {
        return surface_capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actual_extent.width = simple_clamp(actual_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        actual_extent.height = simple_clamp(actual_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

VkImageView create_depth_resources(Device* device, VkExtent2D image_size, VkDeviceMemory& depth_image_memory, VkImage& depth_image)
{
    VkFormat depth_formating = Texture::find_depth_formats(device->physical_device);

    Texture::create_image(device, image_size, depth_formating, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);

    return Texture::create_image_view(device->virtual_device ,depth_image, depth_formating, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkSurfaceFormatKHR select_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats){

    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR select_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

SwapChain create_swap_chain(GLFWwindow* window, Device* device, VkSurfaceKHR surface, SwapChain& swap_chain){
    VkSwapchainKHR khr_swap_chain;
    SwapChainSupportDetails swap_chain_support = find_swap_chain_support(device->physical_device, surface);

    VkSurfaceFormatKHR surface_format = select_swap_surface_format(swap_chain_support.surface_formats);
    VkPresentModeKHR present_mode = select_swap_present_mode(swap_chain_support.surface_present_modes);

    VkExtent2D screen_extent = select_swap_chain_extent(swap_chain_support.surface_capabilities, window);

    VkFormat swap_chain_image_format = surface_format.format;

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

    QueueFamilyIndicies indices = find_queue_families(device->physical_device, surface);
    uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
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

    VkResult result = vkCreateSwapchainKHR(device->virtual_device, &create_info, nullptr, &khr_swap_chain);

    if(result != VK_SUCCESS){
        assert(false && "Failed to create swap chain");
    }
    swap_chain.swap_chain = khr_swap_chain;
    swap_chain.screen_extent = screen_extent;
    swap_chain.swap_chain_image_format = swap_chain_image_format;

    return swap_chain;
}

int clean_swap_chain(VkDevice& virtual_device, SwapChain& swap_chain, SwapChainImages& swap_chain_images)
{
    vkDestroyImageView(virtual_device, swap_chain_images.depth_image_view, nullptr);
    vkDestroyImage(virtual_device, swap_chain_images.depth_image, nullptr);
    vkFreeMemory(virtual_device, swap_chain_images.depth_image_memory, nullptr);

    for (auto framebuffer : swap_chain_images.swap_chain_framebuffers) {
        vkDestroyFramebuffer(virtual_device, framebuffer, nullptr);
    }

    for (auto imageView : swap_chain_images.swap_chain_image_view) {
        vkDestroyImageView(virtual_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(virtual_device, swap_chain.swap_chain, nullptr);
    return 1;
}

void create_swap_chain_images(SwapChain& swap_chain, Device* device, VkSurfaceKHR surface, SwapChainImages& swap_images)
{
    SwapChainSupportDetails swap_chain_support = find_swap_chain_support(device->physical_device, surface);

    uint32_t image_amount = swap_chain_support.surface_capabilities.minImageCount + 1;

    swap_images.swap_chain_images.resize(image_amount);
    vkGetSwapchainImagesKHR(device->virtual_device, swap_chain.swap_chain, &image_amount, swap_images.swap_chain_images.data());

    create_image_views(swap_images, device->virtual_device, swap_chain.swap_chain_image_format);
}

void create_image_views(SwapChainImages& swap_images, VkDevice virtual_device, VkFormat image_format){
    swap_images.swap_chain_image_view.resize(swap_images.swap_chain_images.size());
    
    for (size_t i = 0; i < swap_images.swap_chain_images.size(); i++)
    {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swap_images.swap_chain_images[i];

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


        if(vkCreateImageView(virtual_device, &create_info, nullptr, &swap_images.swap_chain_image_view[i]) != VK_SUCCESS){
            assert(false);
        }
    }
}

void create_frame_buffers(SwapChainImages& swap_images, VkDevice virtual_device, VkRenderPass& render_pass, VkImageView depth_image_view, VkExtent2D extent)
{
    swap_images.swap_chain_framebuffers.resize(swap_images.swap_chain_images.size());

    for (size_t i = 0; i < swap_images.swap_chain_framebuffers.size(); i++) {
        VkImageView attachments[2] = {
            swap_images.swap_chain_image_view[i],
            depth_image_view
        };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = sizeof(attachments) / sizeof(attachments[1]);
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = extent.width;
        framebuffer_info.height = extent.height;
        framebuffer_info.layers = 1;

        if(vkCreateFramebuffer(virtual_device, &framebuffer_info, nullptr, &swap_images.swap_chain_framebuffers[i]) != VK_SUCCESS){
            assert(false);
        }
    }
}

void RenderPass::start_render_pass(VkCommandBuffer& command_buffer, VkFramebuffer frame_buffer, VkRenderPass render_pass, VkExtent2D viewport_extent)
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

//draw_frame(command_buffer, descriptor_set, pipeline_layout, render_buffer, index_amount);
    
}

void RenderPass::end_render_pass(VkCommandBuffer& command_buffer)
{
    if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS){
        assert(false);
    }
}