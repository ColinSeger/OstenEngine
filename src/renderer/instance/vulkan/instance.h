#pragma once
#include "../../../common_includes.h"
namespace Instance
{
    VkInstance create_instance(const char* name, uint32_t window_extention_count, const char** window_extensions, const std::vector<const char*>& validation_layers);

    static bool check_validation_layer_support(const std::vector<const char*>& validation_layers); 
};
