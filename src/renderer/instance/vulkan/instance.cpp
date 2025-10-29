#include "instance.h"

Instance::Instance(const char* name, const bool enable_validation)
{
    //DEBUG REASONS
    if(enable_validation){
        assert(check_validation_layer_support() == true && "Validation layers requested but could not be found");        
    }

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    //Optional setup like name and version number
    app_info.pApplicationName = name;
    app_info.pEngineName = "No Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t glfw_extention_count = 0;
    const char** glfw_extensions;
    
    //Gets critical extensions
    glfw_extensions = glfwGetRequiredInstanceExtensions(& glfw_extention_count);

    create_info.enabledExtensionCount = glfw_extention_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    if(enable_validation){
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }else{
        create_info.enabledLayerCount = 0;
    }
    
    assert(vkCreateInstance(&create_info, nullptr, &instance) == VK_SUCCESS && "Failed to create instance");
    /*
    uint32_t extensions_count = 0;
    std::vector<VkExtensionProperties> extensions(extensions_count);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, extensions.data());

    std::cout << "available extensions:\n";

    for (const auto& extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
    */
}

Instance::~Instance()
{
    vkDestroyInstance(instance, nullptr);
}

bool Instance::check_validation_layer_support()
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

    return true;
}
namespace Debug{
    
    
}

