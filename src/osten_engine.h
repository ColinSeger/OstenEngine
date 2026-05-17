#pragma once
#include <stdint.h>
#include <vulkan/vulkan.h>
#include "../external/RGFW.h"
//#include <GLFW/glfw3.h>
//#include <vulkan/vulkan_core.h>
#include "../external/math_3d.h"
#include "platform.h"
#include "additional_things/arena.h"
//#include "renderer/render_pipeline.h"
// #include "editor/UI/editor_gui.hpp"
//#include "editor/file_explorer/file_explorer.hpp"
//#include "engine/entity_manager/components.h"
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
    //struct RenderPipeline render_pipeline;

    RGFW_window* main_window;
    vec3_t target_point;

    uint8_t* buffer;
    RGFW_surface* surface;

    uint32_t window_width;
    uint32_t window_height;

    //VkDescriptorSet imgui_texture[MAX_LIGHTS];

    //VkInstance instance;

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

} OstenEngine;

static Timer start_time2;
static Timer last_tick;

static inline void create_osten_engine(const uint32_t width, const uint32_t height, const char* application_name, OstenEngine* engine){
    init_mem_arena(&engine->heap_stack, 256 * MB);

    engine->window_height = height;
    engine->window_width = width;

    engine->main_window = RGFW_createWindow(application_name, 0, 0, width, height, RGFW_windowCenter);
    unsigned long long arena_index = arena_alloc_memory(&engine->heap_stack, (uint32_t)(width * height * 4));
    engine->buffer = (uint8_t*)get_at_index(&engine->heap_stack, arena_index);

    engine->surface = RGFW_createSurface(engine->buffer, width, height, RGFW_formatRGBA8);

    setup_renderer(width, height);
}

static inline void draw_frame(OstenEngine* engine){
    engine->should_close = RGFW_window_shouldClose(engine->main_window);
    RGFW_pollEvents();
    //glfwPollEvents();
    RGFW_event window_event = {};
    while (RGFW_window_checkEvent(engine->main_window, &window_event)) {
        switch (window_event.type) {
            case RGFW_eventNone:{

            }
            case RGFW_windowResized:{
                engine->window_width = window_event.update.w;
                engine->window_height = window_event.update.h;
                RGFW_surface_free(engine->surface);

                RGFW_createSurface(engine->buffer, engine->window_width, engine->window_height, RGFW_formatRGBA8);
            }
        }
    }

    engine->delta_time = platform_calc_elapsed_time_seconds(last_tick);
    double frame_time = platform_calc_elapsed_time_seconds(start_time2);

    if (engine->delta_time > 0.20) engine->delta_time = 0.20;

    if(frame_time > 1) {
        //update_graph(platform_memory_mb());

        engine->fps = engine->frames / frame_time;
        start_time2 = platform_get_time_handle();
        engine->frames = 0;
    }

    render_frame(engine->buffer, engine->window_width, engine->window_height);

    RGFW_window_blitSurface(engine->main_window, engine->surface);

    engine->frames +=1;
    last_tick = platform_get_time_handle();
}

static inline void cleanup(OstenEngine* engine){

    RGFW_surface_free(engine->surface);
    destroy_arena(&engine->heap_stack);
}
