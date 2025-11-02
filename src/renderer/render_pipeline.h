#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <fstream>
#include "instance/vulkan/instance.h"
#include "device/vulkan/device.h"
#include "swap_chain/vulkan/swap_chain.h"

class RenderPipeline
{
private:
    //Window to render to
    GLFWwindow* main_window = nullptr;
    //The Vulkan instance
    Instance* instance = nullptr;
    //Device manager
    Device* device = nullptr;

    SwapChain* swap_chain = nullptr;
    
    VkSurfaceKHR surface;

    std::vector<char> load_shader(const std::string& file_name);

    VkShaderModule create_shader(const std::vector<char>& code);

public:
    RenderPipeline(const int width, const int height, const char* application_name);
    ~RenderPipeline();

    void cleanup();

    GLFWwindow* get_main_window(){ return main_window; }
};
