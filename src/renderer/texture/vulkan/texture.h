#pragma once
#include "../../../common_includes.h"
#include "../../device/vulkan/device.h"
#include "../../../../external/image_loader/stb_image.h"

struct TextureImage
{
    VkImage texture_image;
    VkImageView image_view;
    VkSampler texture_sampler;
    VkDeviceMemory texture_image_memory;
};
namespace Texture
{
    VkImage create_texture_image(Device& device ,const char* texture_location, VkCommandPool& command_pool);

    void create_image
    (
        Device& device,
        VkExtent2D& image_size,
        VkFormat format,
        VkImageTiling image_tiling,
        VkImageUsageFlags usage_flags,
        VkMemoryPropertyFlags property_flags,
        VkImage& image,
        VkDeviceMemory& image_memory
    );

    static void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_image_layout, VkImageLayout new_image_layout, Device& device, VkCommandPool& command_pool);

    static void copy_buffer_to_image(VkBuffer buffer, VkImage image, VkExtent2D& image_size, Device& device, VkCommandPool& command_pool);

    VkImageView create_image_view(VkDevice virtual_device, VkImage texture_image, VkFormat texture_format, VkImageAspectFlags image_aspect_flag);

    VkSampler create_texture_sampler(Device& device);
    VkFormat find_depth_formats(VkPhysicalDevice physical_device);

    static VkFormat find_supported_texture_formats(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physical_device);
}
