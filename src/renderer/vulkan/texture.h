#pragma once
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <vulkan/vulkan_core.h>
#include "device.h"
#include "../../../external/stb_image.h"

/**
    This is the representation of the textures used by the engine.
*/
typedef struct TextureImage{
    VkImage texture_image;
    VkImageView image_view;
    VkSampler texture_sampler;
    VkDeviceMemory texture_image_memory;
    uint8_t mip_levels;
} TextureImage;

#define texture_capacity  20

/**
    The storage of the textures used by the engine
*/
typedef struct TextureStorage{
    TextureImage texture;
    uint32_t index;
    char name[255];
    uint8_t name_lenght;
} TextureStorage;

static TextureStorage texture_storage[texture_capacity] = {};
static uint32_t texture_amount = 0;

typedef struct Formats{
    VkFormat* formats;
    uint32_t format_amount;
} Formats;

static inline VkFormat find_supported_texture_formats(const Formats formats, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physical_device){
    for(uint32_t i = 0; i < formats.format_amount; i++){
        VkFormatProperties props = {};
        vkGetPhysicalDeviceFormatProperties(physical_device, formats.formats[i], &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return formats.formats[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return formats.formats[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
    //throw "failed to find supported format!";
}

static inline void transition_image_layout_mip(VkImage* image, VkFormat format, VkImageLayout old_image_layout, VkImageLayout new_image_layout, Device* device, VkCommandPool command_pool, uint8_t mip_level){
    VkCommandBuffer command_buffer = begin_single_time_commands(device->virtual_device, command_pool);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_image_layout;
    barrier.newLayout = new_image_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = *image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mip_level;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_image_layout== VK_IMAGE_LAYOUT_UNDEFINED && new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        return;
        //throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage, destination_stage,
        0,
        0, 0,
        0, 0,
        1, &barrier
    );

    end_single_time_commands(device->virtual_device, command_pool, device->graphics_queue, command_buffer);
}

static inline void copy_buffer_to_image(VkBuffer buffer, VkImage image, VkExtent2D image_size, Device* device, VkCommandPool command_pool)
{
    VkCommandBuffer command_buffer = begin_single_time_commands(device->virtual_device, command_pool);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    VkOffset3D image_offset = {0, 0, 0};
    region.imageOffset = image_offset;

    VkExtent3D image_extent = {
        image_size.width,
        image_size.height,
        1
    };

    region.imageExtent = image_extent;

    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_commands(device->virtual_device, command_pool, device->graphics_queue, command_buffer);
}

static inline VkResult create_image(
                    Device* device,
                    VkExtent2D image_size,
                    VkFormat format,
                    VkImageTiling image_tiling,
                    VkImageUsageFlags usage_flags,
                    VkMemoryPropertyFlags property_flags,
                    VkImage* image,
                    VkDeviceMemory* image_memory
                )
{

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = image_size.width;
    image_info.extent.height = image_size.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = image_tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage_flags;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(device->virtual_device, &image_info, 0, image);

    if(result != VK_SUCCESS)
        return result;

    VkMemoryRequirements memory_requirements = {};
    vkGetImageMemoryRequirements(device->virtual_device, *image, &memory_requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(device->physical_device, memory_requirements.memoryTypeBits, property_flags);

    result = vkAllocateMemory(device->virtual_device, &alloc_info, 0, image_memory);

    if(result != VK_SUCCESS)
        return result;

    return vkBindImageMemory(device->virtual_device, *image, *image_memory, 0);
}

static inline VkImageView create_image_view(VkDevice virtual_device, VkImage texture_image, VkFormat texture_format, VkImageAspectFlags image_aspect_flag)
{
    VkImageView result = {};
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = texture_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = texture_format;
    view_info.subresourceRange.aspectMask = image_aspect_flag;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkResult creation_status = vkCreateImageView(virtual_device, &view_info, 0, &result);

    if(creation_status != VK_SUCCESS)
        return 0;
        //throw("Failed to create image view");

    return result;
}

static inline VkSampler create_texture_sampler(Device* device)
{
    VkSampler textureSampler = {};

    VkPhysicalDeviceProperties properties = {};//TODO Move this out later
    vkGetPhysicalDeviceProperties(device->physical_device, &properties);

    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0;
    sampler_info.maxLod = VK_LOD_CLAMP_NONE;

    VkResult sampler_status = vkCreateSampler(device->virtual_device, &sampler_info, 0, &textureSampler);

    if(sampler_status != VK_SUCCESS){
        return 0;
        //throw("Failed to create texture sampler");
    }

    return textureSampler;
}

static inline VkFormat find_depth_formats(VkPhysicalDevice physical_device) {
    VkFormat valid_formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    Formats formats = {
        formats.formats = valid_formats,
        formats.format_amount = 3
    };

    return find_supported_texture_formats(
        formats,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        physical_device
    );
}

static inline void create_image_simpler(
                    Device* device,
                    VkExtent2D image_size,
                    VkFormat format,
                    VkImageTiling image_tiling,
                    VkImageUsageFlags usage_flags,
                    VkMemoryPropertyFlags property_flags,
                    TextureImage* image
                )
{

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = image_size.width;
    image_info.extent.height = image_size.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = image->mip_levels;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = image_tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage_flags;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(device->virtual_device, &image_info, 0, &image->texture_image);

    if(result != VK_SUCCESS)
        return;
        //throw("Failed to create image");

    VkMemoryRequirements memory_requirements = {};
    vkGetImageMemoryRequirements(device->virtual_device, image->texture_image, &memory_requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(device->physical_device, memory_requirements.memoryTypeBits, property_flags);

    result = vkAllocateMemory(device->virtual_device, &alloc_info, 0, &image->texture_image_memory);

    if(result != VK_SUCCESS)
        return;
        //throw("Failed allocate memory image");

    vkBindImageMemory(device->virtual_device, image->texture_image, image->texture_image_memory, 0);
}

static inline VkImageView create_image_view_mip(VkDevice virtual_device, VkImage texture_image, VkFormat texture_format, VkImageAspectFlags image_aspect_flag, uint8_t mip_level)
{
    VkImageView result = {};
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = texture_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = texture_format;
    view_info.subresourceRange.aspectMask = image_aspect_flag;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = mip_level;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkResult creation_status = vkCreateImageView(virtual_device, &view_info, 0, &result);

    if(creation_status != VK_SUCCESS)
        return 0;
        //throw("Failed to create image view");

    return result;
}

static inline void generate_mipmap(VkImage image, VkFormat image_format, VkExtent2D image_size, uint32_t mip_level, Device* device, VkCommandPool command_pool){
    // Check if image format supports linear blitting
    VkFormatProperties format_properties = {};
    vkGetPhysicalDeviceFormatProperties(device->physical_device, image_format, &format_properties);

    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        //Could Generate mipmaps in other ways later
        return;
        //throw "texture image format does not support linear blitting!";
    }

    VkCommandBuffer command_buffer = begin_single_time_commands(device->virtual_device, command_pool);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mip_width = image_size.width;
    int32_t mip_height = image_size.height;

    for (uint32_t i = 1; i < mip_level; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            0,
            0,
            0,
            1,
            &barrier
        );
        VkOffset3D offset_zeroes = {};
        VkOffset3D offset_two = {mip_width, mip_height, 1};
        VkOffset3D offset_destination = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
        VkImageBlit blit = {};
        blit.srcOffsets[0] = offset_zeroes;
        blit.srcOffsets[1] = offset_two;
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = offset_zeroes;
        blit.dstOffsets[1] = offset_destination;
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(
            command_buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blit,
            VK_FILTER_LINEAR
        );

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            0,
            0,
            0,
            1,
            &barrier
        );
        if (mip_width > 1) mip_width /= 2;
        if (mip_height> 1) mip_height /= 2;
    }
    barrier.subresourceRange.baseMipLevel = mip_level - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, 0,
        0, 0,
        1, &barrier
    );
    end_single_time_commands(device->virtual_device, command_pool, device->graphics_queue ,command_buffer);
}

static inline TextureImage create_texture_image(Device* device ,const char* texture_location, VkCommandPool command_pool){
    int texture_width = 0;
    int texture_height = 0;
    int texture_channels = 0;

    stbi_uc* image_pixels = stbi_load(texture_location, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    VkDeviceSize image_size;
    VkExtent2D image_sizing;
    uint8_t mip_level;
    bool error_handle = false;


    uint8_t image_bit_size = 4;
    if(!image_pixels){
        error_handle = true;

        uint8_t color[] = {255, 105, 180, 255};//HotPink
        image_pixels = color;
        image_size = image_bit_size;
        image_sizing.width = 1;
        image_sizing.height = 1;
        mip_level = 1;
    }else{
        image_size = texture_width * texture_height * image_bit_size;
        image_sizing.width = texture_width;
        image_sizing.height = texture_height;
        mip_level = (uint8_t)(floor(log2(fmax(texture_width, texture_height)))) + 1;
    }


    VkBuffer staging_buffer = {};
    VkDeviceMemory staging_buffer_memory = {};

    create_buffer(
        device,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        image_size,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_buffer_memory
    );

    void* data;
    vkMapMemory(device->virtual_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, image_pixels, (size_t)(image_size));
    vkUnmapMemory(device->virtual_device, staging_buffer_memory);

    TextureImage texture_image = {};
    texture_image.mip_levels = mip_level;

    create_image_simpler(
        device,
        image_sizing,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &texture_image
    );

    transition_image_layout_mip(&texture_image.texture_image,  VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, command_pool, mip_level);

    copy_buffer_to_image(staging_buffer, texture_image.texture_image, image_sizing, device, command_pool);

    //transition_image_layout(texture_image.texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, command_pool, mip_level);

    generate_mipmap(texture_image.texture_image, VK_FORMAT_R8G8B8A8_SRGB, image_sizing, mip_level, device, command_pool);

    if(!error_handle){
        stbi_image_free(image_pixels);
    }

    vkDestroyBuffer(device->virtual_device, staging_buffer, 0);

    return texture_image;
}

static inline TextureStorage load_texture(Device* device, char* texture_location, VkCommandPool command_pool){

    for(uint32_t i = 0; i < texture_amount; i++){
        if(!strcmp(texture_storage[i].name, texture_location)){
            return texture_storage[i];
        }
    }
    TextureImage texture_image = create_texture_image(device, texture_location, command_pool);
    texture_image.image_view = create_image_view_mip(device->virtual_device, texture_image.texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture_image.mip_levels);
    texture_image.texture_sampler = create_texture_sampler(device);
    uint8_t lenght = 0;
    for(int i = 0; i < 255; i++){
        if(texture_location[i] == '.'){
            lenght+=4;//BAD :)
            break;
        }
        lenght++;
    }

    texture_storage[texture_amount].texture = texture_image;
    memcpy(texture_storage[texture_amount].name, texture_location, lenght);
    texture_storage->name_lenght = lenght;
    texture_storage[texture_amount].index = texture_amount;
    texture_amount++;
    return texture_storage[texture_amount -1];
}
