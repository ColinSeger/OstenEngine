#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include "renderer/instance/vulkan/instance.h"
#include "renderer/device/vulkan/device.h"
#include "renderer/swap_chain/vulkan/swap_chain.h"

class Application
{
    const char* application_name = nullptr;
    GLFWwindow* main_window = nullptr;
    Instance* instance = nullptr;
    Device* device = nullptr;
    SwapChain* swap_chain = nullptr;
    
    VkSurfaceKHR surface;

    void create_surface();

public:

    Application(const int width, const int height, const char* name);

    ~Application();
    void main_game_loop();

    void cleanup();
};