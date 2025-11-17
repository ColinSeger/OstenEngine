#include "swap_chain.h"

SwapChain::SwapChain(GLFWwindow* window, VkPhysicalDevice physical_device, VkSurfaceKHR& surface_reference, VkDevice virtual_device) : surface { surface_reference }, virtual_device { virtual_device }
{
    SwapChainSupportDetails swap_chain_support = Setup::find_swap_chain_support(physical_device, surface);

    VkSurfaceFormatKHR surface_format = select_swap_surface_format(swap_chain_support.surface_formats);
    VkPresentModeKHR present_mode = select_swap_present_mode(swap_chain_support.surface_present_modes);

    screen_extent = Setup::select_swap_chain_extent(swap_chain_support.surface_capabilities, window);

    swap_chain_image_format = surface_format.format;

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

    QueueFamilyIndicies indices = Setup::find_queue_families(physical_device, surface);
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

    assert(vkCreateSwapchainKHR(virtual_device, &create_info, nullptr, &swap_chain) == VK_SUCCESS && "Failed to create swap chain");

    vkGetSwapchainImagesKHR(virtual_device, swap_chain, &image_amount, nullptr);
    swap_chain_images.resize(image_amount);
    vkGetSwapchainImagesKHR(virtual_device, swap_chain, &image_amount, swap_chain_images.data());

    create_image_views();
    
}
SwapChain::~SwapChain()
{
    for (auto image_view : swap_chain_image_view) {
        vkDestroyImageView(virtual_device, image_view, nullptr);
    }
    for (auto framebuffer : swap_chain_framebuffers) {
        vkDestroyFramebuffer(virtual_device, framebuffer, nullptr);
    }
    vkDestroySwapchainKHR(virtual_device, swap_chain, nullptr);
}


VkSurfaceFormatKHR SwapChain::select_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats){

    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkExtent2D Setup::select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities, GLFWwindow* window) {
    
    if (surface_capabilities.currentExtent.width != UINT32_MAX) {
        return surface_capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actual_extent.width = glm::clamp(actual_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        actual_extent.height = glm::clamp(actual_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

VkPresentModeKHR SwapChain::select_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChain::create_image_views(){
    swap_chain_image_view.resize(swap_chain_images.size());
    
    for (size_t i = 0; i < swap_chain_images.size(); i++)
    {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swap_chain_images[i];

        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swap_chain_image_format;

        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;


        assert(vkCreateImageView(virtual_device, &create_info, nullptr, &swap_chain_image_view[i]) == VK_SUCCESS);
    }
}

void SwapChain::create_frame_buffers(VkRenderPass& render_pass, VkImageView depth_image_view)
{
    swap_chain_framebuffers.resize(swap_chain_image_view.size());

    for (size_t i = 0; i < swap_chain_image_view.size(); i++) {
        VkImageView attachments[2] = {
            swap_chain_image_view[i],
            depth_image_view
        };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = sizeof(attachments) / sizeof(attachments[1]);
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = screen_extent.width;
        framebuffer_info.height = screen_extent.height;
        framebuffer_info.layers = 1;

        assert(vkCreateFramebuffer(virtual_device, &framebuffer_info, nullptr, &swap_chain_framebuffers[i]) == VK_SUCCESS);
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

void draw_frame(VkCommandBuffer& command_buffer, VkDescriptorSet descriptor_set, VkPipelineLayout pipeline_layout, RenderBuffer& render_buffer, const uint32_t index_amount)
{
    VkBuffer vertex_buffers[] = {render_buffer.vertex_buffer};
    VkDeviceSize offsets[] = {0};
    if(index_amount > 0){
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(command_buffer, render_buffer.index_buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

        vkCmdDrawIndexed(command_buffer, index_amount, 1, 0, 0, 0); 
    }
    

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer, nullptr);

    vkCmdEndRenderPass(command_buffer);
}

void SwapChain::bind_pipeline(VkCommandBuffer& command_buffer, VkPipeline pipeline, VkPipelineLayout pipeline_layout, VkDescriptorSet descriptor_set, RenderBuffer& render_buffer, const uint32_t index_amount)
{
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(screen_extent.width);
    viewport.height = static_cast<float>(screen_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = screen_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    draw_frame(command_buffer, descriptor_set, pipeline_layout, render_buffer, index_amount);
    
}

void RenderPass::end_render_pass(VkCommandBuffer& command_buffer)
{
    assert(vkEndCommandBuffer(command_buffer) == VK_SUCCESS);
}

namespace Setup
{
    QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface){
        QueueFamilyIndicies indices;
        // Logic to find queue family indices to populate struct with
        uint32_t queue_family_amount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_amount, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_amount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_amount, queue_families.data());
        

        int index = 0;
        for (const auto& queue_family : queue_families) {
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics_family = index;
            }
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &present_support);
            if (present_support) {
                indices.present_family = index;
            }

            index++;
        }

        return indices;
    }

    SwapChainSupportDetails find_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR& surface){
        SwapChainSupportDetails swap_chain_details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swap_chain_details.surface_capabilities);

        uint32_t format_amount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_amount, nullptr);

        if (format_amount != 0) {
            swap_chain_details.surface_formats.resize(format_amount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_amount, swap_chain_details.surface_formats.data());
        }

        uint32_t present_mode_amount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_amount, nullptr);

        if (present_mode_amount != 0) {
            swap_chain_details.surface_present_modes.resize(present_mode_amount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_amount, swap_chain_details.surface_present_modes.data());
        }

        return swap_chain_details;
    }
}