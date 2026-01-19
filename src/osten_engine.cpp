#pragma once
#include <cstdint>
#include <chrono>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "platform.h"
#include "additional_things/arena.h"
#include "renderer/instance/vulkan/instance.cpp"
#include "renderer/render_pipeline.cpp"
#include "editor/UI/editor_gui.cpp"
#include "editor/file_explorer/file_explorer.cpp"
#include "engine/entity_manager/components.cpp"
#include "engine/message_system/message.cpp"
#define MATH_3D_IMPLEMENTATION
#include "../external/math_3d.h"

struct OstenEngine
{
    GLFWwindow* main_window = nullptr;
    const char* application_name = "";
    struct RenderPipeline render_pipeline;

    FileExplorer file_explorer;

    MemArena memory_arena;

    bool resized = false;
    static void resize_callback(GLFWwindow* main_window, int width, int height) {
        auto app = reinterpret_cast<OstenEngine*>(glfwGetWindowUserPointer(main_window));
        app->resized = true;
    }

    OstenEngine(const int width, const int height, const char* name);

    ~OstenEngine();
    void main_game_loop();

    void cleanup();
};

OstenEngine::OstenEngine(const int width, const int height, const char* name) : application_name { name }
{
    memory_arena = init_mem_arena(1024 * 1024);
    if(!glfwInit()){
        puts("glfwInit failed");
        throw("GLFW Failed to open");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);
    glfwSetWindowUserPointer(main_window, this);
    glfwSetFramebufferSizeCallback(main_window, resize_callback);

    uint32_t glfw_extention_count = 0;

    // //Gets critical extensions
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extention_count);

    WindowExtentions window_extentions{
        glfw_extensions,
        glfw_extention_count
    };

    //Investigate Why so slow
    VkInstance instance = Instance::create_instance(application_name, window_extentions, memory_arena);

    VkSurfaceKHR surface;

    VkResult result = glfwCreateWindowSurface(instance, main_window, nullptr, &surface);

    if(result != VK_SUCCESS){
        throw("Failed to create surface");
    }

    this->render_pipeline = RenderPipeline(VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, instance, surface, memory_arena);

    init_imgui(main_window, &render_pipeline, memory_arena);

    file_explorer = init_file_explorer();
}

OstenEngine::~OstenEngine()
{
    cleanup();
}

void OstenEngine::main_game_loop()
{
    bool open_window = true;
    static auto start_time = std::chrono::high_resolution_clock::now();
    double frames = 0;

    double fps = 0;

    auto last_tick = std::chrono::high_resolution_clock::now();


    create_transform_system(100, &memory_arena);
    create_camera_system(1, &memory_arena);
    create_render_component_system(50, &memory_arena);

    Message default_texture{
        0,
        MessageType::LoadTexture,
        (void*)".png"
    };
    add_message(default_texture);

    Message empty_entity{
        0,
        MessageType::CreateEntity,
        (void*)"Test"
    };

    add_message(empty_entity);

    uint32_t inspecting = 0;

    VkDescriptorSet imgui_texture = VK_NULL_HANDLE;

    imgui_texture =
    ImGui_ImplVulkan_AddTexture(
        render_pipeline.shadow_pass.debug_sampler,
        render_pipeline.shadow_pass.image_view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();

        auto current_time = std::chrono::high_resolution_clock::now();
        double delta_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - last_tick).count();
        double frame_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - start_time).count();

        handle_message(&render_pipeline, memory_arena);

        if(frame_time > 1)
        {
            update_graph(platform_memory_mb());

            fps = frames / frame_time;
            start_time = current_time;
            frames = 0;
        }

        begin_imgui_editor_poll(main_window, &render_pipeline, open_window, fps, inspecting);
        //ImGui::DockSpaceOverViewport();
        start_file_explorer(file_explorer, &render_pipeline);

        end_file_explorer();
        vkDeviceWaitIdle(render_pipeline.device.virtual_device);
        if(imgui_texture != VK_NULL_HANDLE){
            ImGui::Begin("Viewport");
            ImGui::Image(
                (ImTextureID)imgui_texture,
                ImVec2(256, 256)
            );
            ImGui::End();
        }

        ComponentSystem* cameras = get_component_system(0);
        int32_t result = 0;
        for (size_t i = 0; i < cameras->amount; i++)
        {
            result = render_pipeline.draw_frame(*static_cast<CameraComponent*>(get_component_by_id(cameras, i)), imgui_texture, memory_arena);

            if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized){
                resized = false;
                int32_t width = 0;
                int32_t height = 0;
                glfwGetFramebufferSize(main_window, &width, &height);
                while (width <= 0 || height <= 0) {
                    glfwGetFramebufferSize(main_window, &width, &height);
                    glfwWaitEvents();
                }
                restart_swap_chain(render_pipeline, VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, memory_arena);
                result = render_pipeline.draw_frame(*static_cast<CameraComponent*>(get_component_by_id(cameras, i)), imgui_texture, memory_arena);
            }
        }

        end_imgui_editor_poll();

        frames +=1;
        last_tick = std::chrono::high_resolution_clock::now();
    }
}

void OstenEngine::cleanup()
{
    VkInstance inst = render_pipeline.my_instance;
    VkSurfaceKHR surf = render_pipeline.my_surface;

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplVulkan_Shutdown();
    vkDestroySurfaceKHR(inst, surf, nullptr);
    render_cleanup(render_pipeline, memory_arena);
    vkDestroyInstance(inst, nullptr);
    ImGui::DestroyContext();

    destroy_arena(memory_arena);
}
