#pragma once
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cassert>
#include <vector>
#include <iostream>
#include <cstring>

namespace Instance
{
    VkInstance create_instance(const char* name, const std::vector<const char*>& validation_layers);

    bool check_validation_layer_support(const std::vector<const char*>& validation_layers); 
};
