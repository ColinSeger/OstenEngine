#pragma once
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <vector>
#include <iostream>
#include <cstring>

class Instance
{
    VkInstance instance;

    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
    bool check_validation_layer_support(); 

public:
    Instance(const char* name, const bool enable_validation);
    ~Instance();

    VkInstance& get_instance() { return instance; }
};


namespace Debug
{

     
}
