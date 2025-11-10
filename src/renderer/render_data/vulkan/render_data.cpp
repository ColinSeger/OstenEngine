#include "render_data.h"



void create_descriptor_set_layout(VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout)
{
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &ubo_layout_binding;

    assert(vkCreateDescriptorSetLayout(virtual_device, &layoutInfo, nullptr, &descriptor_set_layout) == VK_SUCCESS);
}



namespace VertexFunctions{

    void create_buffer(Device* device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory)
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        assert(vkCreateBuffer(device->get_virtual_device(), &buffer_info, nullptr, &buffer) == VK_SUCCESS && "Buffer Creation Failed");

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device->get_virtual_device(), buffer, &memory_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = memory_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(device->get_physical_device(), memory_requirements.memoryTypeBits, properties);

        assert(vkAllocateMemory(device->get_virtual_device(), &alloc_info, nullptr, &buffer_memory) == VK_SUCCESS && "Buffer Memory Allocation Failed");

        vkBindBufferMemory(device->get_virtual_device(), buffer, buffer_memory, 0);
    }

    void create_vertex_buffer(Device* device, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory, VkCommandPool& command_pool)
    {
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        create_buffer(
            device,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            buffer_size,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, 
            staging_buffer_memory
        );

        void* data;
        vkMapMemory(device->get_virtual_device(), staging_buffer_memory, 0, buffer_size, 0, &data);
        std::memcpy(data, vertices.data(), (size_t) buffer_size);
        vkUnmapMemory(device->get_virtual_device(), staging_buffer_memory);

        create_buffer(
            device,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            buffer_size,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertex_buffer, 
            vertex_buffer_memory
        );

        CommandBuffer::copy_buffer(device, staging_buffer, vertex_buffer, buffer_size, command_pool);

        vkDestroyBuffer(device->get_virtual_device(),staging_buffer, nullptr);
        vkFreeMemory(device->get_virtual_device(), staging_buffer_memory, nullptr);
    }

    void create_index_buffer(Device* device, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory, VkCommandPool& command_pool)
    {
        VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        create_buffer(
            device,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            buffer_size,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer, 
            staging_buffer_memory
        );

        void* data;
        vkMapMemory(device->get_virtual_device(), staging_buffer_memory, 0, buffer_size, 0, &data);
        std::memcpy(data, indices.data(), (size_t) buffer_size);
        vkUnmapMemory(device->get_virtual_device(), staging_buffer_memory);

        create_buffer(
            device,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            buffer_size,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            index_buffer, 
            index_buffer_memory
        );

        CommandBuffer::copy_buffer(device, staging_buffer, index_buffer, buffer_size, command_pool);

        vkDestroyBuffer(device->get_virtual_device(), staging_buffer, nullptr);
        vkFreeMemory(device->get_virtual_device(), staging_buffer_memory, nullptr);
    }

    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) 
    {
        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
            if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        assert(false && "failed to find suitable memory type!");
    }

    inline VkVertexInputBindingDescription get_binding_description()
    {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }


    inline std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, position);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        return attribute_descriptions;
    }    
}


VkCommandBuffer CommandBuffer::begin_single_time_commands(VkDevice virtual_device, VkCommandPool command_pool)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(virtual_device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &beginInfo);

    return command_buffer;
}

void CommandBuffer::end_single_time_commands(VkDevice virtual_device, VkCommandPool command_pool, VkQueue graphics_queue, VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(virtual_device, command_pool, 1, &command_buffer);
}

void CommandBuffer::copy_buffer(Device* device, VkBuffer& src_buffer, VkBuffer& dst_buffer, VkDeviceSize& size, VkCommandPool& command_pool)
{
    VkCommandBuffer command_buffer = begin_single_time_commands(device->get_virtual_device(), command_pool);

    VkBufferCopy copy_region{};
    //Look into merging copied buffers into this using a offset
    copy_region.srcOffset = 0; // Optional
    copy_region.dstOffset = 0; // Optional
    copy_region.size = size;

    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    end_single_time_commands(device->get_virtual_device(), command_pool, device->get_graphics_queue(), command_buffer);
}