#pragma once
#include <assert.h>
#include <vulkan/vulkan.h>
#include <stdint.h>
#include <string.h>
#include "../../validation.h"
#include "vulkan/vulkan_core.h"

typedef struct WindowExtensions{
    const char** window_extensions;
    uint32_t extensions_amount;
} WindowExtensions;

static inline bool check_validation_layer_support(uint32_t layer_count){
    assert(layer_count < 255);
    VkLayerProperties layers_buffer[255];
    vkEnumerateInstanceLayerProperties(&layer_count, layers_buffer);
    bool layer_found = false;

    for(uint8_t i = 0; i < validation_amount; i++){
        for(uint32_t layer_index = 0; layer_index < layer_count; layer_index++){
            if(strcmp(validation_layers[i], layers_buffer[layer_index].layerName) == 0){
                layer_found = true;
                break;
            }
        }
    }

    if(!layer_found){
        return false;
    }
    return true;
}

static inline VkResult create_instance(VkInstance* instance, const char* name, WindowExtensions window_extensions){
    if(window_extensions.window_extensions == 0) return VK_ERROR_EXTENSION_NOT_PRESENT;
    if(validation_amount > 0){
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, 0);
        if(!check_validation_layer_support(layer_count)){
            return VK_ERROR_VALIDATION_FAILED;//Validation layers requested but could not be found
        }
    }

    VkApplicationInfo app_info = {};

    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    //Optional setup like name and version number
    app_info.pApplicationName = name;
    app_info.pEngineName = "No Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion = VK_API_VERSION_1_4;
    app_info.pNext = 0;

    // VkInstanceCreateFlags flags = {};



    // VkFlags test = VkInstanceCreateFlags();

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.flags = 0;

    create_info.enabledExtensionCount = window_extensions.extensions_amount;
    create_info.ppEnabledExtensionNames = window_extensions.window_extensions;

    if(validation_amount > 0){
        create_info.enabledLayerCount = (uint32_t)validation_amount;
        create_info.ppEnabledLayerNames = validation_layers;
    }else{
        create_info.enabledLayerCount = 0;
    }

    return vkCreateInstance(&create_info, 0, instance);
}
