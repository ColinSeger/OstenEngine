#pragma once
#include <cstdint>
#include <vector>
#include <chrono>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "renderer/instance/vulkan/instance.cpp"
#include "renderer/render_pipeline.cpp"
#include "editor/UI/editor_gui.cpp"
#include "editor/file_explorer/file_explorer.cpp"
#include "engine/entity_manager/components.cpp"
#include "engine/message_system/message.cpp"
#define MATH_3D_IMPLEMENTATION
#include "../external/math_3d.h"

struct PlatformLayer
{
    float (*MemoryChecker)();
};

struct OstenEngine
{
    GLFWwindow* main_window = nullptr;
    const char* application_name = nullptr;
    RenderPipeline* render_pipeline = nullptr;

    FileExplorer file_explorer;

    PlatformLayer& platform_layer;

    bool resized = false;
    static void resize_callback(GLFWwindow* main_window, int width, int height) {
        auto app = reinterpret_cast<OstenEngine*>(glfwGetWindowUserPointer(main_window));
        app->resized = true;
    }


    OstenEngine(const int width, const int height, const char* name, PlatformLayer& layer);

    ~OstenEngine();
    void main_game_loop();

    void cleanup();
};

OstenEngine::OstenEngine(const int width, const int height, const char* name, PlatformLayer& layer) : application_name { name }, platform_layer { layer }
{
    if(!glfwInit()){
        puts("glfwInit failed");
        throw("GLFW Failed to open");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);
    glfwSetWindowUserPointer(main_window, this);
    glfwSetFramebufferSizeCallback(main_window, resize_callback);

    #ifdef NDEBUG
        const char* validation_layers[] = {};
    #else
        const char* validation_layers[] = {
            "VK_LAYER_KHRONOS_validation"
        };
    #endif
    uint8_t validation_layer_amount = sizeof(validation_layers) / sizeof(validation_layers[0]);
    uint32_t glfw_extention_count = 0;

    // //Gets critical extensions
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extention_count);

    VkInstance instance = Instance::create_instance(application_name, glfw_extention_count, glfw_extensions , validation_layers, validation_layer_amount);
    VkSurfaceKHR surface;

    VkResult result = glfwCreateWindowSurface(instance, main_window, nullptr, &surface);

    if(result != VK_SUCCESS){
        throw("Failed to create surface");
    }

    render_pipeline = new RenderPipeline(width, height, instance, surface, validation_layers, validation_layer_amount);

    init_imgui(main_window, render_pipeline);

    file_explorer = init_file_explorer();
}

OstenEngine::~OstenEngine()
{
    cleanup();
}

void shift(std::vector<float>& mem_usage){
    for (int i = 0; i < mem_usage.size() - 1; i++) {
        mem_usage[i] = mem_usage[i+1];
    }
    mem_usage.erase(mem_usage.end());
}

void OstenEngine::main_game_loop()
{
    bool test = true;
    static std::chrono::time_point start_time = std::chrono::high_resolution_clock::now();
    double frames = 0;

    double fps = 0;

    auto last_tick = std::chrono::high_resolution_clock::now();


    create_transform_system(100);
    create_camera_system(1);
    create_render_component_system(50);

    Message empty_entity{
        0,
        MessageType::CreateEntity,
        nullptr
    };
    add_message(empty_entity);
    handle_message(render_pipeline);

    uint32_t inspecting = 0;

    Message default_texture{
        0,
        MessageType::LoadTexture,
        (void*)".."
    };
    add_message(default_texture);

    std::vector<float> mem_usage{};

    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();

        auto current_time = std::chrono::high_resolution_clock::now();
        double delta_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - last_tick).count();
        double frame_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - start_time).count();

        handle_message(render_pipeline);

        if(frame_time > 1)
        {
            mem_usage.push_back(platform_layer.MemoryChecker());
            while(mem_usage.size() > 1000) shift(mem_usage);
            fps = frames / frame_time;
            start_time = current_time;
            frames = 0;
        }

        begin_imgui_editor_poll(main_window, render_pipeline, test, fps, inspecting, mem_usage);
        //ImGui::DockSpaceOverViewport();
        start_file_explorer(file_explorer, render_pipeline);

        end_file_explorer();

        ComponentSystem* cameras = get_component_system(0);
        int32_t result = 0;
        for (size_t i = 0; i < cameras->amount; i++)
        {
            result = render_pipeline->draw_frame(*static_cast<CameraComponent*>(get_component_by_id(cameras, i)));

            if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized){
                resized = false;
                int32_t width = 0, height = 0;
                glfwGetFramebufferSize(main_window, &width, &height);
                while (width <= 0 || height <= 0) {
                    glfwGetFramebufferSize(main_window, &width, &height);
                    glfwWaitEvents();
                }
                render_pipeline->restart_swap_chain(width, height);
                result = render_pipeline->draw_frame(*static_cast<CameraComponent*>(get_component_by_id(cameras, i)));
            }
        }

        end_imgui_editor_poll();

        frames +=1;
        last_tick = std::chrono::high_resolution_clock::now();
    }
}

void OstenEngine::cleanup()
{
    clean_imgui();
    render_pipeline->cleanup();
}
