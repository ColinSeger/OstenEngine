#pragma once
#include <stdint.h>
#include <vulkan/vulkan.h>
#include "../external/RGFW.h"
//#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "../external/math_3d.h"
#include "platform.h"
#include "additional_things/arena.h"
#include "renderer/render_pipeline.h"
// #include "editor/UI/editor_gui.hpp"
//#include "editor/file_explorer/file_explorer.hpp"
#include "engine/entity_manager/components.h"
//#include "renderer/camera/camera.h"
#include "renderer/renderer.h"

#define KB 1024
#define MB  (KB * 1024)
#define GB  (MB * 1024)
/**
    This is the actual engine and is what you would access to modify any data of the engine.

    It contains the render-pipeline and some other data that is there only due to time constraints
*/
typedef struct OstenEngine{
    struct RenderPipeline render_pipeline;

    RGFW_window* main_window;
    vec3_t target_point;

    uint8_t* buffer;
    RGFW_surface* surface;

    //VkDescriptorSet imgui_texture[MAX_LIGHTS];

    VkInstance instance;

    //FileExplorer file_explorer;

    HeapStack heap_stack;

    double frames;

    double fps;

    bool resized;

    bool should_close;

    bool open_window;

    bool paused_update;

    uint32_t inspecting;

    double delta_time;

    // static void resize_callback(GLFWwindow* main_window, int width, int height) {
    //     auto app = (OstenEngine*)(glfwGetWindowUserPointer(main_window));
    //     app->resized = true;
    // }

    //OstenEngine(const uint32_t width, const uint32_t height, const char* name);

    //~OstenEngine();
    //void draw_frame();

    //void cleanup();
} OstenEngine;

static Timer start_time2;
static Timer last_tick;

static inline void create_osten_engine(const uint32_t width, const uint32_t height, const char* application_name, OstenEngine* engine){
    init_mem_arena(&engine->heap_stack, 256 * MB);

    // if(!glfwInit()){
    //     puts("glfwInit failed");
    //     return;
    //     //("GLFW Failed to open");
    // }
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


    engine->main_window = RGFW_createWindow(application_name, 0, 0, width, height, RGFW_windowCenter | RGFW_windowNoResize);
    unsigned long long arena_index = arena_alloc_memory(&engine->heap_stack, (uint32_t)(width * height * 4));
    engine->buffer = (uint8_t*)get_at_index(&engine->heap_stack, arena_index);

    engine->surface = RGFW_createSurface(engine->buffer, width, height, RGFW_formatRGBA8);
    //glfwSetWindowUserPointer(engine->main_window, engine);
    //glfwSetFramebufferSizeCallback(engine->main_window, engine->resize_callback);
    //glfwSetCursorPosCallback(engine->main_window, camera_mouse_callback);
    /*
     *
    VkExtent2D window_size = {(uint32_t)width, (uint32_t)height};

    uint32_t glfw_extension_count = 0;

    // //Gets critical extensions
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    WindowExtensions window_extensions = { glfw_extensions, glfw_extension_count };

    //Investigate Why so slow
    VkResult result = create_instance(&engine->instance, application_name, window_extensions);

    if(result != VK_SUCCESS){
        return;
        //throw "Failed to create Instance";
    }

    VkSurfaceKHR surface;

    result = glfwCreateWindowSurface(engine->instance, engine->main_window, 0, &surface);

    if(result != VK_SUCCESS){
        return;
        //throw "Failed to create surface";
    }

    result = create_render_pipeline(window_size, engine->instance, surface, &engine->render_pipeline, &engine->heap_stack);

    if(result != VK_SUCCESS){
        return;
        //throw "Failed to create render pipeline";
    }
     */

    // init_imgui(main_window, &render_pipeline, instance, &heap_stack);
    // for(uint8_t i = 0; i < MAX_LIGHTS; i++){
    //     imgui_texture[i] = ImGui_ImplVulkan_AddTexture
    //     (
    //         render_pipeline.lights.debug_shadow_sampler,
    //         render_pipeline.lights.shadow_passes[i].image_view,
    //         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    //     );

    // }

    //file_explorer = init_file_explorer();

    create_transform_system(20000, &engine->heap_stack);
    create_camera_system(2, &engine->heap_stack);
    add_camera(add_transform());
    add_camera(add_transform());
    create_render_component_system(9000, &engine->heap_stack);
    create_collider_system(10000, &engine->heap_stack);
    create_health_system(10000, &engine->heap_stack);
    create_melee_system(10000, &engine->heap_stack);
}

// OstenEngine::~OstenEngine(){
//     cleanup();
// }

static inline void draw_frame(OstenEngine* engine){
    engine->should_close = RGFW_window_shouldClose(engine->main_window);

    //glfwPollEvents();


    engine->delta_time = platform_calc_elapsed_time_seconds(last_tick);
    double frame_time = platform_calc_elapsed_time_seconds(start_time2);

    if (engine->delta_time > 0.20) engine->delta_time = 0.20;

    if(frame_time > 1) {
        //update_graph(platform_memory_mb());

        engine->fps = engine->frames / frame_time;
        start_time2 = platform_get_time_handle();
        engine->frames = 0;
    }

    render_frame2(engine->buffer, 1920, 1080);

    RGFW_window_blitSurface(engine->main_window, engine->surface);

    // ImGui::Begin("ShadowMap");
    // for(uint8_t i = 0; i < MAX_LIGHTS; i++){
    //     if(imgui_texture[i] != VK_NULL_HANDLE){
    //         ImGui::Image(
    //             (ImTextureID)imgui_texture[i],
    //             ImVec2(256, 256)
    //         );
    //     }
    // }
    // ImGui::End();
    //camera_movement(engine->delta_time, 0, engine->main_window);
/*
 *
    ComponentSystem* camera_sys = get_component_system(0);

    int32_t result = 0;
    //for (size_t i = 0; i < cameras->amount; i++)
    {
        CameraComponent* camera = (CameraComponent*)get_component_by_id(camera_sys, 0);
        //CameraComponent* light_source = (CameraComponent*)get_component_by_id(camera_sys, 1);
        result = render_frame(&engine->render_pipeline, camera, &engine->heap_stack);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || engine->resized){
            engine->resized = false;
            int32_t width = 0;
            int32_t height = 0;
            glfwGetFramebufferSize(engine->main_window, &width, &height);
            while (width <= 0 || height <= 0) {
                glfwGetFramebufferSize(engine->main_window, &width, &height);
                glfwWaitEvents();
            }
            restart_swap_chain(&engine->render_pipeline, (VkExtent2D){(uint32_t)(width), (uint32_t)(height)}, &engine->heap_stack);
            result = render_frame(&engine->render_pipeline, camera, &engine->heap_stack);
        }
    }
 */

    // end_imgui_editor_poll();

    engine->frames +=1;
    last_tick = platform_get_time_handle();
}

static inline void cleanup(OstenEngine* engine){
    struct RenderPipeline* render_pipeline = &engine->render_pipeline;
    if(!render_pipeline->device.virtual_device)return;
    VkSurfaceKHR surf = render_pipeline->my_surface;

    vkDeviceWaitIdle(render_pipeline->device.virtual_device);

    // ImGui_ImplGlfw_Shutdown();
    // ImGui_ImplVulkan_Shutdown();
    render_cleanup(render_pipeline, &engine->heap_stack);
    vkDestroySurfaceKHR(engine->instance, surf, 0);
    vkDestroyInstance(engine->instance, 0);
    // ImGui::DestroyContext();

    destroy_arena(&engine->heap_stack);
}
