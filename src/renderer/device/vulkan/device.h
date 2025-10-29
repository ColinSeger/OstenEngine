#pragma once
#include <vulkan/vulkan.h>
#include <cassert>
#include <vector>
#include <optional>

struct QueueFamilyIndicies{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
};

class Device
{
private:
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    VkDevice virtual_device;

    VkQueue graphics_queue;

    VkSurfaceKHR& surface_pointer;

    bool is_device_suitable(VkPhysicalDevice device);

    QueueFamilyIndicies find_queue_families(VkPhysicalDevice device);


public:
    Device(VkInstance& instance, VkSurfaceKHR& surface);
    ~Device();
};
