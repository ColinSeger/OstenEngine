#include "device.h"


namespace DeviceHelperFunctions
{
    bool is_completed(QueueFamilyIndicies& queue_family)
    {
        return queue_family.graphics_family.has_value() && queue_family.present_family.has_value();
    }

    bool is_completed(SwapChainSupportDetails& swap_chain_support)
    {
        return !swap_chain_support.surface_formats.empty() && ! swap_chain_support.surface_present_modes.empty();
    }
}
QueueFamilyIndicies find_queue_families(VkPhysicalDevice device, VkSurfaceKHR& surface){
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
        if(indices.graphics_family.has_value() && indices.present_family.has_value()){
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

    if (format_amount != 0) {
        swap_chain_details.surface_formats.resize(format_amount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_amount, swap_chain_details.surface_formats.data());
    }

    uint32_t present_mode_amount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_amount, nullptr);

    if (present_mode_amount != 0) {
        swap_chain_details.surface_present_modes.resize(present_mode_amount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_amount, swap_chain_details.surface_present_modes.data());
    }

    return swap_chain_details;
}


void create_device(Device& device,VkInstance& instance, VkSurfaceKHR& surface_reference, const std::vector<const char*>& validation_layers)
{
    device.surface = surface_reference;
    uint32_t device_amount = 0;

    vkEnumeratePhysicalDevices(instance, &device_amount, nullptr);

    assert(device_amount > 0 && "There is no device that supports vulkan on this computer");

    std::vector<VkPhysicalDevice> devices(device_amount);
    vkEnumeratePhysicalDevices(instance, &device_amount, devices.data());


    for (const VkPhysicalDevice& device_physical : devices) {
        if (is_device_suitable(device_physical, device.surface)) {
            device.physical_device = device_physical;
            break;
        }
    }

    assert(device.physical_device != VK_NULL_HANDLE && "No vulkan supported graphics found");

    //
    ///Creation of virtual device starts here
    //
    create_virtual_device(&device, validation_layers);

}

// Device::~Device()
// {
//     vkDestroyDevice(virtual_device, nullptr);
// }

bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface)//Can improve later
{
    QueueFamilyIndicies indices = find_queue_families(device, surface);

    bool has_extention_support = check_device_extension_support(device);

    bool has_swap_chain_support = false;

    if(has_extention_support){
        SwapChainSupportDetails swap_chain_support = find_swap_chain_support(device, surface);
        has_swap_chain_support = DeviceHelperFunctions::is_completed(swap_chain_support);
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);
    
    return DeviceHelperFunctions::is_completed(indices) && has_extention_support && has_swap_chain_support;
}

bool check_device_extension_support(VkPhysicalDevice device) 
{
    uint32_t extension_count;

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);

    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

void create_virtual_device(Device* device, const std::vector<const char*>& validation_layers)
{
    QueueFamilyIndicies indices = find_queue_families(device->physical_device, device->surface);

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
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();

    uint32_t validation_length = validation_layers.size();
    if (validation_length > 0) {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_length);
        create_info.ppEnabledLayerNames = validation_layers.data();
    } else {
        create_info.enabledLayerCount = 0;
    }
    VkResult result = vkCreateDevice(device->physical_device, &create_info, nullptr, &device->virtual_device);
    if(result != VK_SUCCESS)
    {
        printf("Issue in creation of virtual device (%i)", result);
        assert(false && "Failed to create device");
    }

    vkGetDeviceQueue(device->virtual_device, indices.graphics_family.value(), 0, &device->graphics_queue);
    vkGetDeviceQueue(device->virtual_device, indices.present_family.value(), 0, &device->present_queue);
}


