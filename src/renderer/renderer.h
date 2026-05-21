#pragma once
#include <stdint.h>
#include "../additional_things/arena.h"
#define CLAY_IMPLEMENTATION
#include "../../external/clay.h"
#include "../../external/math_3d.h"

typedef struct OstenWindow{
    uint32_t window_width;
    uint32_t window_height;
    uint8_t* window_buffer;
    uint8_t* window_surface;
} OstenWindow;

typedef enum RenderModes{
    None,
    Rectangle,
    Count
} RenderModes;

typedef struct RendererBoundingBox {
    float x, y, width, height;
} RendererBoundingBox;

typedef struct RenderCommand {
    RendererBoundingBox boundingBox;
    RenderModes commandType;
} RenderCommand;


static Clay_Arena arena;

static uint16_t child_gap = 5;

static void HandleClayErrors(Clay_ErrorData errorData) {}

static inline uint32_t setup_renderer(MemArena* memArena, uint32_t width, uint32_t height){
    uint64_t totalMemorySize = Clay_MinMemorySize();
    arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, (void*)arena_alloc_memory(memArena, totalMemorySize));
    Clay_Initialize(arena, (Clay_Dimensions) { (float)width, (float)height },(Clay_ErrorHandler) { HandleClayErrors });
    return 0;
}

static inline void* get_pixel(uint32_t x, uint32_t y, uint8_t* surface, uint32_t surface_width){
    const uint32_t pixel_size = 4;
    return &surface[y * (pixel_size * surface_width) + (x) * pixel_size];
}

static inline void render_rectangle(OstenWindow* window, Clay_BoundingBox bounds, Clay_Color color){
    for (uint32_t y = bounds.y; y < bounds.height; y++) {
        for(uint32_t x = bounds.x; x < bounds.width; x++){
            uint8_t* pixel = (uint8_t*)get_pixel(x, y, window->window_buffer, window->window_width);
            pixel[0] = color.r;
            pixel[1] = color.g;
            pixel[2] = color.b;
            pixel[3] = color.a;
        }
    }
}

static inline uint32_t render_frame(OstenWindow* window, vec2_t mouse_cords){
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)window->window_width, (float)window->window_height });

    Clay_BeginLayout();

    CLAY(CLAY_ID("MainContainer"), { .layout = { .sizing = {CLAY_SIZING_PERCENT(1.f), CLAY_SIZING_PERCENT(1.f)}, .padding = CLAY_PADDING_ALL(5), .childGap = child_gap }, .backgroundColor = {155,155,155,255} }) {
        CLAY(CLAY_ID("SideBar"), {
            .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.2f), .height = CLAY_SIZING_GROW() }, .padding = CLAY_PADDING_ALL(5), .childGap = child_gap },
            .backgroundColor = (Clay_Color) { 100, 100, 100, 255}
        }) {

            //CLAY(CLAY_ID("MainContent"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } }, .backgroundColor = (Clay_Color) {224, 215, 210, 255} }) {}
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    for (int i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand* renderCommand = &renderCommands.internalArray[i];

        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                render_rectangle(window, renderCommand->boundingBox, renderCommand->renderData.rectangle.backgroundColor);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT:{
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE:{
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER:{
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM:{
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:{
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:{
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_NONE:{
                break;
            }
        }
    }


    render_rectangle(window, (Clay_BoundingBox){ mouse_cords.x, mouse_cords.y, mouse_cords.x + 10, mouse_cords.y + 10 }, (Clay_Color){0,0,0,255});

    return 0;
}

static inline uint32_t cleanup_renderer(){
    return 0;
}
