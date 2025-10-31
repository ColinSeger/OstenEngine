#include "swap_chain.h"

SwapChain::SwapChain(GLFWwindow* window, VkPhysicalDevice physical_device, VkSurfaceKHR& surface_reference, VkDevice virtual_device) : main_window { window }, surface { surface_reference }, virtual_device { virtual_device }
{
    SwapChainSupportDetails swap_chain_support = Setup::find_swap_chain_support(physical_device, surface);

    VkSurfaceFormatKHR surface_format = select_swap_surface_format(swap_chain_support.surface_formats);
    VkPresentModeKHR present_mode = select_swap_present_mode(swap_chain_support.surface_present_modes);
    screen_extent = select_swap_chain_extent(swap_chain_support.surface_capabilities);

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

}
SwapChain::~SwapChain()
{
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

VkExtent2D SwapChain::select_swap_chain_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities) {
    
    if (surface_capabilities.currentExtent.width != UINT32_MAX) {
        return surface_capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(main_window, &width, &height);

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
