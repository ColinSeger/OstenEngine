#pragma once
#include <stdint.h>
#include "../additional_things/arena.h"
#define CLAY_IMPLEMENTATION
#include "../../external/clay.h"
#include "../../external/osten_math.h"

typedef struct OstenWindow {
    uint32_t window_width;
    uint32_t window_height;
    uint8_t* window_buffer;
    uint8_t* window_surface;
} OstenWindow;

typedef enum RenderModes {
    RenderNone,
    RenderRectangle,
    RenderText,
    RenderCount
} RenderType;

typedef struct BoundingBox {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} BoundingBox;


typedef struct OstColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} OstColor;

typedef struct RenderCommand {
    BoundingBox bounding_box;
    RenderType command_type;
    OstColor color;
} RenderCommand;

static uint16_t child_gap = 5;

static inline OstColor clay_to_ost_color(Clay_Color color){
    return (OstColor){(uint8_t)color.r, (uint8_t)color.g, (uint8_t)color.b, (uint8_t)color.a};
}

static inline BoundingBox clay_to_ost_bounding_box(Clay_BoundingBox bounding_box){
    BoundingBox box = {(uint32_t)bounding_box.x, (uint32_t)bounding_box.y, (uint32_t)bounding_box.width, (uint32_t)bounding_box.height};
    return box;
}

static inline RenderCommand clay_to_ost_command(Clay_RenderCommand command){
    RenderCommand result = {};

    switch (command.commandType) {
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {

            result.command_type = RenderRectangle;
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_TEXT:{
            result.command_type = RenderText;
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

    result.bounding_box = clay_to_ost_bounding_box(command.boundingBox);
    result.color = clay_to_ost_color(command.renderData.rectangle.backgroundColor);
    return result;
}

static inline OstColor* get_pixel(uint32_t x, uint32_t y, OstenWindow* window){
    const uint32_t pixel_size = 4;
    return (OstColor*)&window->window_buffer[(y * (pixel_size * window->window_width)) + (x * pixel_size)];
}

static inline void render_rectangle(OstenWindow* window, BoundingBox bounds, OstColor color){

    uint32_t width = 0;
    width = bounds.width - bounds.x;
    if(bounds.width > window->window_width && bounds.x < window->window_width){
        width = window->window_width - bounds.x;
    }

    if(bounds.height > window->window_height){
        if(bounds.y > window->window_height){
            bounds.height = 0;
        }else{
            bounds.height = window->window_height - bounds.y;
        }
    }


    for (uint32_t y = bounds.y; y < bounds.height; y++) {

        OstColor* pixel = get_pixel(bounds.x, y, window);

        for(uint32_t x = 0; x < width; x++){
            pixel[x] = color;
        }
    }
}
static Clay_Arena arena;
static void HandleClayErrors(Clay_ErrorData errorData) {}

void debug_init_clay(OstenWindow* engine_window, MemArena* mem_arena)
{
    uint64_t totalMemorySize = Clay_MinMemorySize();
    arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, (void*)arena_alloc_memory(mem_arena, totalMemorySize));
    Clay_Initialize(arena, (Clay_Dimensions) { (float)engine_window->window_width, (float)engine_window->window_height },(Clay_ErrorHandler) { HandleClayErrors });
}

static inline uint32_t render_frame(OstenWindow* window, vec2 mouse_cords){
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)window->window_width, (float)window->window_height });

    Clay_BeginLayout();

    CLAY(CLAY_ID("MainContainer"), { .layout = { .sizing = {CLAY_SIZING_PERCENT(1.f), CLAY_SIZING_PERCENT(1.f)}, .padding = CLAY_PADDING_ALL(5), .childGap = child_gap }, .backgroundColor = {155,155,155,255} }) {
        CLAY(CLAY_ID("SideBar"), {
            .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.2f), .height = CLAY_SIZING_GROW() }, .padding = CLAY_PADDING_ALL(5), .childGap = child_gap },
            .backgroundColor = (Clay_Color) { 100, 100, 100, 255}
        }) {

        }
    }

    Clay_RenderCommandArray render_commands = Clay_EndLayout();

    for (int i = 0; i < render_commands.length; i++) {
        RenderCommand renderCommand = clay_to_ost_command(render_commands.internalArray[i]);

        switch (renderCommand.command_type) {
            case RenderRectangle: {
                render_rectangle(window, renderCommand.bounding_box, renderCommand.color);

            }break;
            default:
            break;
        }

    }

    //render_rectangle(window, (Clay_BoundingBox){ 0, 0, (float)window->window_width, 10 }, (OstColor){0,0,0,255});

    render_rectangle(window, (BoundingBox){ (uint32_t)mouse_cords.x, (uint32_t)mouse_cords.y, (uint32_t)mouse_cords.x + 10, (uint32_t)mouse_cords.y + 10 }, (OstColor){0,0,0,255});

    return 0;
}

static inline uint32_t cleanup_renderer(){
    return 0;
}
