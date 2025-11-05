#include "render_data.h"

namespace VertexFunctions{

    void create_vertex_buffer(Device* device, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory)
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = sizeof(vertices[0]) * vertices.size();
        buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        assert(vkCreateBuffer(device->get_virtual_device(), &buffer_info, nullptr, &vertex_buffer) == VK_SUCCESS);
        
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(device->get_virtual_device(), vertex_buffer, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(device->get_physical_device(), mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        assert(vkAllocateMemory(device->get_virtual_device(), &alloc_info, nullptr, &vertex_buffer_memory) == VK_SUCCESS);

        vkBindBufferMemory(device->get_virtual_device(), vertex_buffer, vertex_buffer_memory, 0);

        void* data;
        vkMapMemory(device->get_virtual_device(), vertex_buffer_memory, 0, buffer_info.size, 0, &data);
        memcpy(data, vertices.data(), (size_t) buffer_info.size);
        vkUnmapMemory(device->get_virtual_device(), vertex_buffer_memory);
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
