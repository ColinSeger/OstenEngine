#pragma once
#include <filesystem>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "../../device/vulkan/device.h"
#include "../../render_data/vulkan/render_data.h"
#include "../../../../external/image_loader/stb_image.h"

#include <iostream>//Remove

struct TextureImage
{
    VkImage texture_image;
    VkDeviceMemory texture_image_memory;
};

/// @brief Width Height
struct ImageSize
{
    uint32_t width;
    uint32_t height;
};


namespace Texture
{
    VkImage create_texture_image(Device* device ,const char* texture_location, VkCommandPool& command_pool);

    void create_image
    (
        Device* device, 
        ImageSize image_size, 
        VkFormat format, 
        VkImageTiling image_tiling, 
        VkImageUsageFlags usage_flags, 
        VkMemoryPropertyFlags property_flags, 
        VkImage& image, 
        VkDeviceMemory& image_memory
    );

    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_image_layout, VkImageLayout new_image_layout, Device* device, VkCommandPool& command_pool);

    void copy_buffer_to_image(VkBuffer buffer, VkImage image, ImageSize& image_size, Device* device, VkCommandPool& command_pool);

    VkImageView create_image_view(VkDevice virtual_device, VkImage texture_image, VkFormat texture_format, VkImageAspectFlags image_aspect_flag);

    VkSampler create_texture_sampler(Device* device);
    VkFormat find_depth_formats(VkPhysicalDevice physical_device);

    VkFormat find_supported_texture_formats(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physical_device);
}