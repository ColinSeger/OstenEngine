#pragma once
#include "../../../common_includes.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Instance
{
    VkInstance create_instance(const char* name, const std::vector<const char*>& validation_layers);

    bool check_validation_layer_support(const std::vector<const char*>& validation_layers); 
};
