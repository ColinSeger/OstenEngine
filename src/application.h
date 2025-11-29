#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <string>
#include "../external/imgui_test/imgui.h"
#include "../external/imgui_test/imgui_internal.h"
#include "../external/imgui_test/imgui_impl_glfw.h"
#include "../external/imgui_test/imgui_impl_vulkan.h"
#include "debugger/debugger.h"
#include "renderer/texture/vulkan/texture.h"
#include "renderer/render_pipeline.h"
#include "engine/entity_manager/entity_manager.h"

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

class Application
{
    const char* application_name = nullptr;
    GLFWwindow* main_window = nullptr;
    RenderPipeline* render_pipeline = nullptr;

    std::vector<char*> logs;

public:

    Application(const int width, const int height, const char* name);

    ~Application();
    void main_game_loop();

    void move_camera(double delta_time);

    void imgui_hierarchy_pop_up();

    void imgui_hierarchy(bool& open);

    void cleanup();
};