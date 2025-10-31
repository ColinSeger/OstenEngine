#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include "instance/vulkan/instance.h"
#include "device/vulkan/device.h"

class RenderPipeline
{
private:
    const char* application_name = nullptr;
    GLFWwindow* main_window = nullptr;
    Instance* instance = nullptr;
    Device* device = nullptr;
    
    VkSurfaceKHR surface;
public:
    RenderPipeline();
    ~RenderPipeline();
};
