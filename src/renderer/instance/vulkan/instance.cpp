#include "instance.h"

VkInstance Instance::create_instance(const char* name, const std::vector<const char*>& validation_layers)
{
    VkInstance instance = VK_NULL_HANDLE;
    uint16_t layer_size = validation_layers.size();
    if(layer_size > 0){
        if(check_validation_layer_support(validation_layers) == true){
            assert(false && "Validation layers requested but could not be found");
        }       
    }

    VkApplicationInfo app_info{};
    
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    //Optional setup like name and version number
    app_info.pApplicationName = name;
    app_info.pEngineName = "No Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion = VK_API_VERSION_1_4;
    app_info.pNext = nullptr;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.flags = VkInstanceCreateFlags(0);

    uint32_t glfw_extention_count = 0;
    
    //Gets critical extensions
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extention_count);
    assert(glfw_extensions != NULL);

    create_info.enabledExtensionCount = glfw_extention_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    if(layer_size > 0){
        create_info.enabledLayerCount = static_cast<uint32_t>(layer_size);
        create_info.ppEnabledLayerNames = validation_layers.data();
    }else{
        create_info.enabledLayerCount = 0;
    }
    if(vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS){
        assert(false && "Failed to create instance");
    }
    

    return instance;
}

bool Instance::check_validation_layer_support(const std::vector<const char*>& validation_layers)
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for(const char* layer_name : validation_layers){
        bool layer_found = false;

        for(const auto& layer_properties : available_layers){
            if(strcmp(layer_name, layer_properties.layerName) == 0){
                layer_found = true;
                break;
            }
        }
        if(!layer_found){
            return false;
        }
    }
    

    // for(const char* layer_name : validation_layers){
        
    // }

    return true;
}