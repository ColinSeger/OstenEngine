#include "device.h"

//
/// Device is
//
Device::Device(VkInstance& instance, VkSurfaceKHR& surface_reference) : surface { surface_reference }
{
    // surface_pointer = surface;
    uint32_t device_amount = 0;

    vkEnumeratePhysicalDevices(instance, &device_amount, nullptr);

    assert(device_amount > 0 && "There is no device that supports vulkan on this computer");

    std::vector<VkPhysicalDevice> devices(device_amount);
    vkEnumeratePhysicalDevices(instance, &device_amount, devices.data());


    for (const VkPhysicalDevice& device : devices) {
        if (is_device_suitable(device)) {
            physical_device = device;
            break;
        }
    }

    assert(physical_device != VK_NULL_HANDLE && "No vulkan supported graphics found");

    //
    ///Creation of virtual device starts here
    //
    create_virtual_device();

}

Device::~Device()
{
    vkDestroyDevice(virtual_device, nullptr);
}

bool Device::is_device_suitable(VkPhysicalDevice device)//Can improve later
{
    QueueFamilyIndicies indices = find_queue_families(device);
    return indices.graphics_family.has_value() && indices.present_family.has_value();


    //Supposedly gets vulkan version support, name of device and other things
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    //Gets optional features like texture compression
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);

    //Makes it so that device has to be discrete gpu and supports geometry shaders
    return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader;
}

void Device::create_virtual_device()
{
    QueueFamilyIndicies indices = find_queue_families(physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};
    float queuePriority = 1.0f;


    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queuePriority;
        queue_create_infos.push_back(queue_create_info);
    }

    
    // queue_create_info.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = 0;

    // if (enable_validation) {
    //     create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    //     create_info.ppEnabledLayerNames = validationLayers.data();
    // } else {
        create_info.enabledLayerCount = 0;
    // }


    assert(vkCreateDevice(physical_device, &create_info, nullptr, &virtual_device) == VK_SUCCESS);

    vkGetDeviceQueue(virtual_device, indices.graphics_family.value(), 0, &graphics_queue);
    vkGetDeviceQueue(virtual_device, indices.present_family.value(), 0, &present_queue);
}


QueueFamilyIndicies Device::find_queue_families(VkPhysicalDevice device){
    QueueFamilyIndicies indices;
    // Logic to find queue family indices to populate struct with
    uint32_t queue_family_amount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_amount, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_amount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_amount, queue_families.data());

    

    int index = 0;
    for (const auto& queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = index;
        }
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &present_support);
        if (present_support) {
            indices.present_family = index;
        }

        index++;
    }

    return indices;
}