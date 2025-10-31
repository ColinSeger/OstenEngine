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

//
/// Device is
//
Device::Device(VkInstance& instance, VkSurfaceKHR& surface_reference, bool enable_validation) : surface { surface_reference }
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
    create_virtual_device(enable_validation);

}

Device::~Device()
{
    vkDestroyDevice(virtual_device, nullptr);
}

bool Device::is_device_suitable(VkPhysicalDevice device)//Can improve later
{
    QueueFamilyIndicies indices = Setup::find_queue_families(device, surface);

    bool has_extention_support = check_device_extension_support(device);

    bool has_swap_chain_support = false;

    if(has_extention_support){
        SwapChainSupportDetails swap_chain_support = Setup::find_swap_chain_support(device, surface);
        has_swap_chain_support = DeviceHelperFunctions::is_completed(swap_chain_support);
    }
    
    return DeviceHelperFunctions::is_completed(indices) && has_extention_support && has_swap_chain_support;
}

bool Device::check_device_extension_support(VkPhysicalDevice device) 
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

void Device::create_virtual_device(bool enable_validation)
{
    QueueFamilyIndicies indices = Setup::find_queue_families(physical_device, surface);

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

    create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();

    if (enable_validation) {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    } else {
        create_info.enabledLayerCount = 0;
    }


    assert(vkCreateDevice(physical_device, &create_info, nullptr, &virtual_device) == VK_SUCCESS);

    vkGetDeviceQueue(virtual_device, indices.graphics_family.value(), 0, &graphics_queue);
    vkGetDeviceQueue(virtual_device, indices.present_family.value(), 0, &present_queue);
}


