#include "texture.h"


VkImage Texture::create_texture_image(Device* device ,const char* texture_location, VkCommandPool& command_pool)
{
    int texture_width;
    int texture_height;
    int texture_channels;

    stbi_uc* image_pixels = stbi_load(texture_location, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);

    VkDeviceSize image_size = texture_width * texture_height * 4;
    
    if(!image_pixels){
        std::cout << texture_location << "\n";//TODO REMOVE
        
        throw std::runtime_error("std::filesystem::current_path()");
    }
    

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    CommandBuffer::create_buffer(
        device, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        image_size,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory
    );

    void* data;
    vkMapMemory(device->virtual_device, staging_buffer_memory, 0, image_size, 0, &data);
        memcpy(data, image_pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(device->virtual_device, staging_buffer_memory);

    stbi_image_free(image_pixels);

    VkImage texture_image{};
    VkDeviceMemory texture_image_memory;

    VkExtent2D image_sizing;
    image_sizing.width = texture_width; 
    image_sizing.height = texture_height;

    create_image
    (
        device, 
        image_sizing, 
        VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        texture_image, 
        texture_image_memory
    );

    transition_image_layout(texture_image,  VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, command_pool);

    copy_buffer_to_image(staging_buffer, texture_image, image_sizing, device, command_pool);

    transition_image_layout(texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, command_pool);

    return texture_image;
}

void Texture::create_image( Device* device, 
                            VkExtent2D& image_size, 
                            VkFormat format, 
                            VkImageTiling image_tiling, 
                            VkImageUsageFlags usage_flags, 
                            VkMemoryPropertyFlags property_flags, 
                            VkImage& image, 
                            VkDeviceMemory& image_memory
                          )
{

    VkImageCreateInfo image_info{};
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

    VkResult result = vkCreateImage(device->virtual_device, &image_info, nullptr, &image);

    if(result != VK_SUCCESS){
        assert(false && "Failed to create image");
    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(device->virtual_device, image, &memory_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = CommandBuffer::find_memory_type(device->physical_device ,memory_requirements.memoryTypeBits, property_flags);

    result = vkAllocateMemory(device->virtual_device, &alloc_info, nullptr, &image_memory);

    if(result != VK_SUCCESS){
        assert(false && "Failed allocate memory image");
    }

    vkBindImageMemory(device->virtual_device, image, image_memory, 0);
}

void Texture::transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_image_layout, VkImageLayout new_image_layout, Device* device, VkCommandPool& command_pool)
{
    VkCommandBuffer command_buffer = CommandBuffer::begin_single_time_commands(device->virtual_device, command_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_image_layout;
    barrier.newLayout = new_image_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
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
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage, destination_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    CommandBuffer::end_single_time_commands(device->virtual_device, command_pool, device->graphics_queue, command_buffer);
}

void Texture::copy_buffer_to_image(VkBuffer buffer, VkImage image, VkExtent2D& image_size, Device* device, VkCommandPool& command_pool)
{
    VkCommandBuffer command_buffer = CommandBuffer::begin_single_time_commands(device->virtual_device, command_pool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        image_size.width,
        image_size.height,
        1
    };

    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    CommandBuffer::end_single_time_commands(device->virtual_device, command_pool, device->graphics_queue, command_buffer);
}

VkImageView Texture::create_image_view(VkDevice virtual_device, VkImage texture_image, VkFormat texture_format, VkImageAspectFlags image_aspect_flag)
{
    VkImageView result{};
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = texture_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = texture_format;
    view_info.subresourceRange.aspectMask = image_aspect_flag;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if(vkCreateImageView(virtual_device, &view_info, nullptr, &result) != VK_SUCCESS){
        assert(false);
    }

    return result;
}

VkSampler Texture::create_texture_sampler(Device* device)
{
    VkSampler textureSampler{};

    VkPhysicalDeviceProperties properties{};//Move this out later
    vkGetPhysicalDeviceProperties(device->physical_device, &properties);

    VkSamplerCreateInfo sampler_info{};
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
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    if(vkCreateSampler(device->virtual_device, &sampler_info, nullptr, &textureSampler) != VK_SUCCESS){
        assert(false);
    }

    return textureSampler;
}

VkFormat Texture::find_depth_formats(VkPhysicalDevice physical_device) {
    return find_supported_texture_formats(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        physical_device
    );
}

VkFormat Texture::find_supported_texture_formats(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physical_device)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}
