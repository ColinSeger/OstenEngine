#pragma once
#include <string.h>
#include <stdint.h>
#define CLAY_IMPLEMENTATION
#include "../../external/clay.h"

static Clay_Arena arena;

static uint16_t child_gap = 5;

static void HandleClayErrors(Clay_ErrorData errorData) {}

static inline uint32_t setup_renderer(uint32_t width, uint32_t height){
    uint64_t totalMemorySize = Clay_MinMemorySize();
    arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
    Clay_Initialize(arena, (Clay_Dimensions) { (float)width, (float)height },(Clay_ErrorHandler) { HandleClayErrors });
    return 0;
}

static inline void* get_pixel(uint32_t x, uint32_t y, uint8_t* surface, uint32_t surface_width){
    const uint32_t pixel_size = 4;
    const uint32_t screen_width = 1920;
    return &surface[y * (pixel_size * surface_width) + (x) * pixel_size];
}

static inline void render_rectangle(uint8_t* surface, Clay_BoundingBox bounds, Clay_Color color, uint32_t surface_width){
    for (uint32_t y = bounds.y; y < bounds.height; y++) {
        for(uint32_t x = bounds.x; x < bounds.width; x++){
            *(Clay_Color*)get_pixel(x, y, surface, surface_width) = color;
        }
    }
}

static inline uint32_t render_frame(uint8_t* surface, uint32_t width, uint32_t height){
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float)width, (float)height });
    // if(surface[0] <= 255){
    //     memset(surface, surface[0]+1, (width * height) * 4);
    //     //return 1;
    // }

    // if(surface[0] >= 255){
    //     memset(surface, 0, (width * height) * 4);
    //     //return 1;
    // }
    Clay_BeginLayout();

    CLAY(CLAY_ID("MainContainer"), { .layout = { .sizing = {CLAY_SIZING_PERCENT(1.f), CLAY_SIZING_PERCENT(1.f)}, .padding = CLAY_PADDING_ALL(5), .childGap = child_gap }, .backgroundColor = {250,250,255,255} }) {
        CLAY(CLAY_ID("SideBar"), {
            .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.2f), .height = CLAY_SIZING_GROW() }, .padding = CLAY_PADDING_ALL(5), .childGap = child_gap },
            .backgroundColor = (Clay_Color) { 224, 215, 210, 255}
        }) {

            //CLAY(CLAY_ID("MainContent"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } }, .backgroundColor = (Clay_Color) {224, 215, 210, 255} }) {}
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    for (int i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand* renderCommand = &renderCommands.internalArray[i];

        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                render_rectangle(surface, renderCommand->boundingBox, renderCommand->renderData.rectangle.backgroundColor, width);
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT:{

            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE:{

            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER:{

            }
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM:{

            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:{

            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:{

            }
            case CLAY_RENDER_COMMAND_TYPE_NONE:{}
        }
    }
    return 0;
}

static inline uint32_t cleanup_renderer(){
    return 0;
}
