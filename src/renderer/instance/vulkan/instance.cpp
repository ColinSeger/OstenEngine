#pragma once
#include <cstdlib>
#include <vulkan/vulkan.h>
#include <cstdint>
#include <stdint.h>
#include <cstring>

namespace Instance
{
    static bool check_validation_layer_support(const char*const* validation_layers, uint8_t amount)
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        VkLayerProperties* available_layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
        bool layer_found = false;

        for(uint8_t i = 0; i < amount; i++){
            for(uint32_t layer_index = 0; layer_index < layer_count; layer_index++){
                if(strcmp(validation_layers[i], available_layers[layer_index].layerName) == 0){
                    layer_found = true;
                    break;
                }
            }
        }

        free(available_layers);

        if(!layer_found){
            return false;
        }
        return true;
    }

    VkInstance create_instance(const char* name, uint32_t window_extention_count, const char** window_extensions, const char* const* validation_layers, uint8_t layer_amount)
    {
        if(window_extensions == NULL) throw;
        VkInstance instance = VK_NULL_HANDLE;
        if(layer_amount > 0){
            if(!check_validation_layer_support(validation_layers, layer_amount)){
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

        create_info.enabledExtensionCount = window_extention_count;
        create_info.ppEnabledExtensionNames = window_extensions;

        if(layer_amount > 0){
            create_info.enabledLayerCount = static_cast<uint32_t>(layer_amount);
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
