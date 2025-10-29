#pragma once
#include <vulkan/vulkan.h>
#include <cassert>
#include <vector>
#include <optional>

struct QueueFamilyIndicies{
    std::optional<uint32_t> graphics_family;
};

class Device
{
private:
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    bool is_device_suitable(VkPhysicalDevice device);

    QueueFamilyIndicies find_queue_families(VkPhysicalDevice device);


public:
    Device(VkInstance& instance);
    ~Device();
};
