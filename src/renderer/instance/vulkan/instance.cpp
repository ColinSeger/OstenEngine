#include "instance.h"

Instance::Instance(const char* name)
{
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

    create_info.enabledLayerCount = 0;
    assert(vkCreateInstance(&create_info, nullptr, &instance) == VK_SUCCESS && "Failed to create instance");
    
    uint32_t extensions_count = 0;
    std::vector<VkExtensionProperties> extensions(extensions_count);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, extensions.data());

    std::cout << "available extensions:\n";

    for (const auto& extension : extensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

Instance::~Instance()
{
    vkDestroyInstance(instance, nullptr);
}