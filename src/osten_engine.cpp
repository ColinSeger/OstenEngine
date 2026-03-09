#pragma once
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "platform.h"
#include "additional_things/arena.h"
#include "renderer/descriptors/descriptors.h"
#include "renderer/instance/vulkan/instance.h"
#include "renderer/render_pipeline.cpp"
#include "editor/UI/editor_gui.hpp"
#include "editor/file_explorer/file_explorer.hpp"
#include "engine/entity_manager/components.h"
#include "../external/math_3d.h"
#include "renderer/camera/camera.h"

constexpr size_t KB = 1024;
constexpr size_t MB = KB * 1024;
constexpr size_t GB = MB * 1024;

struct OstenEngine
{
    struct RenderPipeline render_pipeline;

    GLFWwindow* main_window = nullptr;
    vec3_t target_point = {};

    VkDescriptorSet imgui_texture[MAX_LIGHTS];

    VkInstance instance;

    FileExplorer file_explorer;

    HeapStack heap_stack;

    double frames = 0;

    double fps = 0;

    bool resized = false;

    bool should_close = false;

    bool open_window = true;

    bool paused_update = false;

    uint32_t inspecting = 0;

    double delta_time = 0;

    static void resize_callback(GLFWwindow* main_window, int width, int height) {
        auto app = reinterpret_cast<OstenEngine*>(glfwGetWindowUserPointer(main_window));
        app->resized = true;
    }

    OstenEngine(const int width, const int height, const char* name);

    ~OstenEngine();
    void draw_frame();

    void cleanup();
};

static Timer start_time2 = platform_get_time_handle();
static Timer last_tick;

OstenEngine::OstenEngine(const int width, const int height, const char* application_name){
    init_mem_arena(&heap_stack, 256*MB);

    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

    if(!glfwInit()){
        puts("glfwInit failed");
        return;
        //("GLFW Failed to open");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);
    glfwSetWindowUserPointer(main_window, this);
    glfwSetFramebufferSizeCallback(main_window, resize_callback);
    glfwSetCursorPosCallback(main_window, camera_mouse_callback);

    VkExtent2D window_size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    uint32_t glfw_extension_count = 0;

    // //Gets critical extensions
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    WindowExtensions window_extensions{ glfw_extensions, glfw_extension_count };

    //Investigate Why so slow
    VkResult result = create_instance(&instance, application_name, window_extensions);

    if(result != VK_SUCCESS){
        return;
        throw "Failed to create Instance";
    }

    VkSurfaceKHR surface;

    result = glfwCreateWindowSurface(instance, main_window, nullptr, &surface);

    if(result != VK_SUCCESS){
        return;
        throw "Failed to create surface";
    }

    result = create_render_pipeline(window_size, instance, surface, this->render_pipeline, &heap_stack);

    if(result != VK_SUCCESS){
        return;
        throw "Failed to create render pipeline";
    }

    init_imgui(main_window, &render_pipeline, instance, &heap_stack);
    for(uint8_t i = 0; i < MAX_LIGHTS; i++){
        imgui_texture[i] = ImGui_ImplVulkan_AddTexture
        (
            render_pipeline.lights.debug_shadow_sampler,
            render_pipeline.lights.shadow_passes[i].image_view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

    }

    file_explorer = init_file_explorer();

    create_transform_system(20000, &heap_stack);
    create_camera_system(2, &heap_stack);
    add_camera(add_transform());
    add_camera(add_transform());
    create_render_component_system(9000, &heap_stack);
    create_collider_system(10000, &heap_stack);
    create_health_system(10000, &heap_stack);
    create_melee_system(10000, &heap_stack);
}

OstenEngine::~OstenEngine(){
    cleanup();
}

void OstenEngine::draw_frame(){
    should_close = glfwWindowShouldClose(main_window);

    glfwPollEvents();


    delta_time = platform_calc_elapsed_time_seconds(last_tick);
    double frame_time = platform_calc_elapsed_time_seconds(start_time2);

    if (delta_time > 0.20) delta_time = 0.20;

    if(frame_time > 1) {
        update_graph(platform_memory_mb());

        fps = frames / frame_time;
        start_time2 = platform_get_time_handle();
        frames = 0;
    }
    ImGui::Begin("ShadowMap");
    for(uint8_t i = 0; i < MAX_LIGHTS; i++){
        if(imgui_texture[i] != VK_NULL_HANDLE){
            ImGui::Image(
                (ImTextureID)imgui_texture[i],
                ImVec2(256, 256)
            );
        }
    }
    ImGui::End();
    camera_movement(delta_time, 0, main_window);

    ComponentSystem* camera_sys = get_component_system(0);

    int32_t result = 0;
    //for (size_t i = 0; i < cameras->amount; i++)
    {
        CameraComponent* camera = (CameraComponent*)get_component_by_id(camera_sys, 0);
        //CameraComponent* light_source = (CameraComponent*)get_component_by_id(camera_sys, 1);
        result = render_pipeline.draw_frame(*camera, &heap_stack);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized){
            resized = false;
            int32_t width = 0;
            int32_t height = 0;
            glfwGetFramebufferSize(main_window, &width, &height);
            while (width <= 0 || height <= 0) {
                glfwGetFramebufferSize(main_window, &width, &height);
                glfwWaitEvents();
            }
            restart_swap_chain(&render_pipeline, VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, &heap_stack);
            result = render_pipeline.draw_frame(*camera, &heap_stack);
        }
    }

    end_imgui_editor_poll();

    frames +=1;
    last_tick = platform_get_time_handle();
}

void OstenEngine::cleanup(){
    VkSurfaceKHR surf = render_pipeline.my_surface;

    vkDeviceWaitIdle(render_pipeline.device.virtual_device);

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplVulkan_Shutdown();
    render_cleanup(render_pipeline, &heap_stack);
    vkDestroySurfaceKHR(instance, surf, nullptr);
    vkDestroyInstance(instance, nullptr);
    ImGui::DestroyContext();

    destroy_arena(&heap_stack);
}
