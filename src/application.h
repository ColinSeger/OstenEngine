#pragma once
#include "common_includes.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <string>
#include <unordered_map>
#include "renderer/texture/vulkan/texture.h"
#include "renderer/render_pipeline.h"
#include "engine/entity_manager/entity_manager.h"
#include "editor/file_explorer/file_explorer.h"
#include "engine/entity_manager/entity_system.h"
#include "engine/entity_manager/components.h"


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
    GLFWwindow* main_window = nullptr;
    const char* application_name = nullptr;
    RenderPipeline* render_pipeline = nullptr;

    std::vector<char*> logs;
    
    std::vector<System> systems;

    FileExplorer file_explorer;

    bool resized = false;
    static void resize_callback(GLFWwindow* main_window, int width, int height) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(main_window));
        app->resized = true;
    }
public:

    Application(const int width, const int height, const char* name);

    ~Application();
    void main_game_loop();

    void move_camera(double delta_time);

    void imgui_hierarchy_pop_up();

    void imgui_hierarchy(bool& open);

    void cleanup();
};