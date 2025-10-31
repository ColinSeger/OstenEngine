#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include "renderer/render_pipeline.h"

class Application
{
    const char* application_name = nullptr;
    GLFWwindow* main_window = nullptr;
    RenderPipeline* render_pipeline = nullptr;

public:

    Application(const int width, const int height, const char* name);

    ~Application();
    void main_game_loop();

    void cleanup();
};