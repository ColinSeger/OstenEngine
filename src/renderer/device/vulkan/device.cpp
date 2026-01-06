#pragma once
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vulkan/vulkan.h>
#include "../../../../external/math_3d.h"
#include "../../../debugger/debugger.cpp"
#include "vulkan/vulkan_core.h"

struct TextureCord{
    float x = 0;
    float y = 0;
};

struct Vertex {
    vec3_t position{};
    vec3_t color{};
    TextureCord texture_cord{};
};

typedef struct{
    size_t amount;
    Vertex* values;
}VertexArray;

typedef struct{
    size_t amount;
    uint32_t* values;
} Uint32Array;

typedef struct{
    VkVertexInputAttributeDescription array[3];
} VertexAttributes;

struct optional
{
    uint32_t number;
    bool has_value = false;
};

typedef struct{
    optional graphics_family;
    optional present_family;
}  QueueFamilyIndicies;

typedef struct{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    void* surface_data;
    //VkSurfaceFormatKHR* surface_formats;
    //VkPresentModeKHR* surface_present_modes;
    uint8_t surface_amount;
    uint8_t present_amount;
} SwapChainSupportDetails ;

struct Device
{
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice virtual_device = VK_NULL_HANDLE;

    VkQueue graphics_queue;
    VkQueue present_queue;
};

static const char* device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


constexpr bool is_completed(const QueueFamilyIndicies& queue_family)
{
    return queue_family.graphics_family.has_value && queue_family.present_family.has_value;
}

bool is_completed(SwapChainSupportDetails& swap_chain_support)
{
    return swap_chain_support.present_amount > 0 && swap_chain_support.surface_amount > 0;
}

VkVertexInputBindingDescription get_binding_description() {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_description;
}

VertexAttributes get_attribute_descriptions() {
    VertexAttributes attribute_descriptions{};
    attribute_descriptions.array[0].binding = 0;
    attribute_descriptions.array[0].location = 0;
    attribute_descriptions.array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions.array[0].offset = offsetof(Vertex, position);

    attribute_descriptions.array[1].binding = 0;
    attribute_descriptions.array[1].location = 1;
    attribute_descriptions.array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions.array[1].offset = offsetof(Vertex, color);

    attribute_descriptions.array[2].binding = 0;
    attribute_descriptions.array[2].location = 2;
    attribute_descriptions.array[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions.array[2].offset = offsetof(Vertex, texture_cord);

    return attribute_descriptions;
}

QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface){
    QueueFamilyIndicies indices;
    // Logic to find queue family indices to populate struct
    uint32_t queue_family_amount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_amount, nullptr);

    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queue_family_amount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_amount, queue_families);

    int index = 0;
    for (int i = 0; i < queue_family_amount; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family.number = index;
            indices.graphics_family.has_value = true;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &present_support);

        if (present_support) {
            indices.present_family.number = index;
            indices.present_family.has_value = true;
        }
        if(indices.graphics_family.has_value && indices.present_family.has_value){
            break;
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

    uint32_t present_mode_amount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_amount, nullptr);

    swap_chain_details.surface_amount = format_amount;
    swap_chain_details.present_amount = present_mode_amount;


    swap_chain_details.surface_data = malloc((sizeof(VkSurfaceFormatKHR) * format_amount) + (sizeof(VkPresentModeKHR) * present_mode_amount));
    //VkSurfaceFormatKHR surfaces[format_amount];// This NEEDS to be turned into a actual allocation since this will be deallocated after function call
    //VkPresentModeKHR presents[present_mode_amount];

    if (format_amount != 0) {
        //swap_chain_details.surface_formats = surfaces;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_amount, (VkSurfaceFormatKHR*)swap_chain_details.surface_data);
    }

    if (present_mode_amount != 0) {
        VkSurfaceFormatKHR* surface_end = (VkSurfaceFormatKHR*)swap_chain_details.surface_data;
        surface_end+= format_amount;
        VkPresentModeKHR* offset = (VkPresentModeKHR*)surface_end;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_amount, offset);
    }

    return swap_chain_details;
}

static bool check_device_extension_support(VkPhysicalDevice device)
{
    uint32_t extension_count = 0;

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    VkExtensionProperties* available_extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_count);//I Dislike this

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions);

    uint32_t extension_amount = sizeof(device_extensions) / sizeof(device_extensions[0]);
    uint32_t found_amount = 0;

    for (int i = 0; i < extension_count; i++) {
        //Improve Later
        if(!strcmp(device_extensions[0], available_extensions[i].extensionName)){
            found_amount++;
        }
    }
    free(available_extensions);
    if(found_amount >= extension_amount){
        return true;
    }else{
        return false;
    }
    // return required_extensions.empty();
}

static bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface)//Can improve later
{
    QueueFamilyIndicies indices = find_queue_families(device, surface);

    bool has_extention_support = check_device_extension_support(device);

    bool has_swap_chain_support = false;

    if(has_extention_support){
        SwapChainSupportDetails swap_chain_support = find_swap_chain_support(device, surface);
        has_swap_chain_support = is_completed(swap_chain_support);
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);

    return is_completed(indices) && has_extention_support && has_swap_chain_support;
}

static inline bool contains(VkDeviceQueueCreateInfo queue_create_infos[],const uint32_t compare, const uint32_t amount)
{
    for (int i = 0; i < amount; i++) {
        if(queue_create_infos[i].queueFamilyIndex == compare){
            return true;
        }
    }
    return false;
}

static void create_virtual_device(Device& device, VkSurfaceKHR surface, const char* const* validation_layers, uint8_t layer_amount)
{
    QueueFamilyIndicies indices = find_queue_families(device.physical_device, surface);
    uint32_t family_array[] = {indices.graphics_family.number, indices.present_family.number};

    VkDeviceQueueCreateInfo queue_create_infos[sizeof(family_array) / sizeof(family_array[0])];
    uint8_t queue_create_info_amount = 0;

    float queuePriority = 1.0f;

    for (int i = 0; i < sizeof(queue_create_infos) / sizeof(queue_create_infos[0]); i++) {
        queue_create_infos[i].queueFamilyIndex = UINT32_MAX;
    }

    for (uint32_t queue_family : family_array) {
        if(contains(queue_create_infos, queue_family, sizeof(queue_create_infos) / sizeof(queue_create_infos[0]))) continue;

        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queuePriority;
        queue_create_infos[queue_create_info_amount] = queue_create_info;
        queue_create_info_amount++;
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_info_amount);
    create_info.pQueueCreateInfos = queue_create_infos;

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = static_cast<uint32_t>(sizeof(device_extensions) / sizeof(device_extensions[0]));
    create_info.ppEnabledExtensionNames = device_extensions;

    if (layer_amount > 0) {
        create_info.enabledLayerCount = static_cast<uint32_t>(layer_amount);
        create_info.ppEnabledLayerNames = validation_layers;
    } else {
        create_info.enabledLayerCount = 0;
    }
    VkResult result = vkCreateDevice(device.physical_device, &create_info, nullptr, &device.virtual_device);

    if(result != VK_SUCCESS)
    {
        Debug::log((char*)"Issue in creation of virtual device ");

        throw "Failed to create device";
    }

    vkGetDeviceQueue(device.virtual_device, indices.graphics_family.number, 0, &device.graphics_queue);
    vkGetDeviceQueue(device.virtual_device, indices.present_family.number, 0, &device.present_queue);
}

void create_device(Device& device,VkInstance& instance, VkSurfaceKHR& surface_reference, const char* const* validation_layers, uint8_t layer_amount)
{
    uint32_t device_amount = 0;

    vkEnumeratePhysicalDevices(instance, &device_amount, nullptr);

    if(device_amount <= 0){
        throw "There is no device that supports vulkan on this computer";
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * device_amount);
    vkEnumeratePhysicalDevices(instance, &device_amount, devices);

    for (int i = 0; i < device_amount; i++) {
        if (is_device_suitable(devices[i], surface_reference)) {
            device.physical_device = devices[i];
            break;
        }
    }

    if(device.physical_device == VK_NULL_HANDLE){
        throw "No vulkan supported graphics found";
    }

    create_virtual_device(device, surface_reference, validation_layers, layer_amount);
}

void destroy_device(Device& device)
{
    vkDestroyDevice(device.virtual_device, nullptr);
}

namespace CommandBuffer
{
    VkCommandBuffer begin_single_time_commands(VkDevice virtual_device, VkCommandPool& command_pool)
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

    void end_single_time_commands(VkDevice virtual_device, VkCommandPool& command_pool, VkQueue graphics_queue, VkCommandBuffer& command_buffer)
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

    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
            if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        Debug::log((char*)"Failed to find memory");
        throw("failed to find suitable memory type!");
        return 0;
    }

    static void copy_buffer(Device& device, VkBuffer& src_buffer, VkBuffer& dst_buffer, VkDeviceSize& size, VkCommandPool& command_pool)
    {
        VkCommandBuffer command_buffer = begin_single_time_commands(device.virtual_device, command_pool);

        VkBufferCopy copy_region{};
        //Look into merging copied buffers into this using a offset
        copy_region.srcOffset = 0; // Optional
        copy_region.dstOffset = 0; // Optional
        copy_region.size = size;

        vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

        end_single_time_commands(device.virtual_device, command_pool, device.graphics_queue, command_buffer);
    }

    void create_buffer(Device& device, VkBufferUsageFlags usage, VkDeviceSize size, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory)
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device.virtual_device, &buffer_info, nullptr, &buffer);

        if(result != VK_SUCCESS){
            throw("Failed to create buffer");
        }

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device.virtual_device, buffer, &memory_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = memory_requirements.size;
        alloc_info.memoryTypeIndex = find_memory_type(device.physical_device, memory_requirements.memoryTypeBits, properties);

        result = vkAllocateMemory(device.virtual_device, &alloc_info, nullptr, &buffer_memory);

        if(result != VK_SUCCESS){
            throw("Buffer Memory Allocation Failed");
        }

        vkBindBufferMemory(device.virtual_device, buffer, buffer_memory, 0);
    }

    void create_vertex_buffer(Device& device, VertexArray& vertices, VkBuffer& vertex_buffer, VkDeviceMemory& vertex_buffer_memory, VkCommandPool& command_pool)
    {
        VkDeviceSize buffer_size = sizeof(vertices.values[0]) * vertices.amount;

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
        vkMapMemory(device.virtual_device, staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, vertices.values, (size_t) buffer_size);
        vkUnmapMemory(device.virtual_device, staging_buffer_memory);

        create_buffer(
            device,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            buffer_size,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertex_buffer,
            vertex_buffer_memory
        );

        CommandBuffer::copy_buffer(device, staging_buffer, vertex_buffer, buffer_size, command_pool);

        vkDestroyBuffer(device.virtual_device,staging_buffer, nullptr);
        vkFreeMemory(device.virtual_device, staging_buffer_memory, nullptr);
    }

    void create_index_buffer(Device& device, Uint32Array& indicies, VkBuffer& index_buffer, VkDeviceMemory& index_buffer_memory, VkCommandPool& command_pool)
    {
        VkDeviceSize buffer_size = sizeof(indicies.values[0]) * indicies.amount;

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
        vkMapMemory(device.virtual_device, staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, indicies.values, (size_t) buffer_size);
        vkUnmapMemory(device.virtual_device, staging_buffer_memory);

        create_buffer(
            device,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            buffer_size,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            index_buffer,
            index_buffer_memory
        );

        CommandBuffer::copy_buffer(device, staging_buffer, index_buffer, buffer_size, command_pool);

        vkDestroyBuffer(device.virtual_device, staging_buffer, nullptr);
        vkFreeMemory(device.virtual_device, staging_buffer_memory, nullptr);
    }


    void record_command_buffer(VkCommandBuffer& command_buffer)
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0; // Optional
        begin_info.pInheritanceInfo = nullptr; // Optional


        VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
        if(result != VK_SUCCESS){
            throw("Failed at recording command buffer");
        }
    }

    //Can probably make this a array
    void create_command_buffers(VkCommandBuffer* command_buffers, VkDevice virtual_device, VkCommandPool& command_pool, const uint8_t MAX_FRAMES_IN_FLIGHT)
    {
        VkCommandBufferAllocateInfo allocation_info{};
        allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocation_info.commandPool = command_pool;
        allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocation_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

        VkResult result = vkAllocateCommandBuffers(virtual_device, &allocation_info, command_buffers);
        if(result != VK_SUCCESS){
            throw("Failed at creating command buffers");
        }
    }

    VkCommandPool create_command_pool(Device& device, VkSurfaceKHR surface)
    {
        VkCommandPool command_pool;
        QueueFamilyIndicies queue_family_indices = find_queue_families(device.physical_device, surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queue_family_indices.graphics_family.number;

        VkResult result = vkCreateCommandPool(device.virtual_device, &poolInfo, nullptr, &command_pool);
        if(result != VK_SUCCESS){
            throw("Failed at Creating command pool");
        }
        return command_pool;
    }
}
