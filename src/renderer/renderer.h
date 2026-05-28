#pragma once
#include <stdint.h>
#include "../additional_things/arena.h"
#define CLAY_IMPLEMENTATION
#include "../../external/clay.h"
#include "../../external/math_3d.h"

typedef struct OstenWindow {
    uint32_t window_width;
    uint32_t window_height;
    uint8_t* window_buffer;
    uint8_t* window_surface;
} OstenWindow;

typedef enum RenderModes {
    None,
    Rectangle,
    Count
} RenderModes;

typedef struct BoundingBox {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} Boundingbox;

typedef struct RenderCommand {
    Boundingbox boundingBox;
    RenderModes commandType;
} RenderCommand;

typedef struct OstColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} OstColor;

static uint16_t child_gap = 5;

static inline OstColor clay_to_ost_color(Clay_Color color){
    return (OstColor){(uint8_t)color.r, (uint8_t)color.g, (uint8_t)color.b, (uint8_t)color.a};
}

static inline Boundingbox clay_to_ost_bounding_box(Clay_BoundingBox bounding_box){
    Boundingbox box = {(uint32_t)bounding_box.x, (uint32_t)bounding_box.y, (uint32_t)bounding_box.width, (uint32_t)bounding_box.height};
    return box;
}

static inline uint32_t setup_renderer(MemArena* memArena, uint32_t width, uint32_t height){

    return 0;
}

static inline OstColor* get_pixel(uint32_t x, uint32_t y, OstenWindow* window){
    const uint32_t pixel_size = 4;
    return (OstColor*)&window->window_buffer[(y * (pixel_size * window->window_width)) + (x * pixel_size)];
}

static inline void render_rectangle(OstenWindow* window, Boundingbox bounds, OstColor color){
    for (uint32_t y = bounds.y; y < bounds.height; y++) {

        OstColor* pixel = get_pixel(bounds.x, y, window);

        for(uint32_t x = 0; x < bounds.width - bounds.x; x++){
            pixel[x] = color;
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

        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    for (int i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand* renderCommand = &renderCommands.internalArray[i];

        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {

                render_rectangle(window, clay_to_ost_bounding_box(renderCommand->boundingBox), clay_to_ost_color(renderCommand->renderData.rectangle.backgroundColor));
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

    //render_rectangle(window, (Clay_BoundingBox){ 0, 0, (float)window->window_width, 10 }, (OstColor){0,0,0,255});

    render_rectangle(window, (Boundingbox){ (uint32_t)mouse_cords.x, (uint32_t)mouse_cords.y, (uint32_t)mouse_cords.x + 10, (uint32_t)mouse_cords.y + 10 }, (OstColor){0,0,0,255});

    return 0;
}

static inline uint32_t cleanup_renderer(){
    return 0;
}
