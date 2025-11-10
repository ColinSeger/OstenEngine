#pragma once
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <vector>
#include <iostream>
#include <cstring>

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};
namespace Instance
{
    VkInstance create_instance(const char* name, const bool enable_validation);

    bool check_validation_layer_support(); 
};
