#pragma once
#include <cstddef>
#include <cstdlib>
#include <vulkan/vulkan.h>
#include <cstdint>
#include <stdint.h>
#include <cstring>
#include "../../../additional_things/arena.h"
#include "../../validation.h"

typedef struct {
    const char** window_extensions;
    uint32_t extensions_amount;
} WindowExtentions;

namespace Instance
{
    static bool check_validation_layer_support(MemArena& memory_arena)
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        size_t index = arena_alloc_memory(memory_arena, sizeof(VkLayerProperties) * layer_count);
        VkLayerProperties* available_layers = (VkLayerProperties*)memory_arena[index];
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
        bool layer_found = false;

        for(uint8_t i = 0; i < validation_amount; i++){
            for(uint32_t layer_index = 0; layer_index < layer_count; layer_index++){
                if(strcmp(validation_layers[i], available_layers[layer_index].layerName) == 0){
                    layer_found = true;
                    break;
                }
            }
        }

        free_arena(memory_arena, index);

        if(!layer_found){
            return false;
        }
        return true;
    }

    VkInstance create_instance(const char* name, WindowExtentions window_extensions, MemArena& memory_arena)
    {
        if(window_extensions.window_extensions == NULL) throw;
        VkInstance instance = VK_NULL_HANDLE;
        if(validation_amount > 0){
            if(!check_validation_layer_support(memory_arena)){
                throw("Validation layers requested but could not be found");
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

        create_info.enabledExtensionCount = window_extensions.extensions_amount;
        create_info.ppEnabledExtensionNames = window_extensions.window_extensions;

        if(validation_amount > 0){
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_amount);
            create_info.ppEnabledLayerNames = validation_layers;
        }else{
            create_info.enabledLayerCount = 0;
        }
        if(vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS){
            throw("Failed to create instance");
        }


        return instance;
    }
};
