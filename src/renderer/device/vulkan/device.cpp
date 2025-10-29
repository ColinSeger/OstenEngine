#include "device.h"

Device::Device(VkInstance& instance)
{
    uint32_t device_amount = 0;

    vkEnumeratePhysicalDevices(instance, &device_amount, nullptr);

    assert(device_amount <= 0 && "There is no device that supports vulkan on this computer");

    std::vector<VkPhysicalDevice> devices(device_amount);
    vkEnumeratePhysicalDevices(instance, &device_amount, devices.data());


    for (const VkPhysicalDevice& device : devices) {
        if (is_device_suitable(device)) {
            physical_device = device;
            break;
        }
    }

    assert(physical_device == VK_NULL_HANDLE && "No vulkan supported graphics found");
}

Device::~Device()
{
}

bool Device::is_device_suitable(VkPhysicalDevice device)//Can improve later
{
    return true;
    //Supposedly gets vulkan version support, name of device and other things
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    //Gets optional features like texture compression
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);

    //Makes it so that device has to be discrete gpu and supports geometry shaders
    return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader;
}

QueueFamilyIndicies Device::find_queue_families(VkPhysicalDevice device){
    QueueFamilyIndicies indices;
    // Logic to find queue family indices to populate struct with
    uint32_t queue_family_amount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_amount, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_amount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_amount, queue_families.data());

    return indices;
}