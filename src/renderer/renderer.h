#pragma once
#include <string.h>
#include <stdint.h>
#define CLAY_IMPLEMENTATION
#include "../../external/clay.h"

static Clay_Arena arena;

static void HandleClayErrors(Clay_ErrorData errorData) {}

static inline uint32_t setup_renderer(){
    uint64_t totalMemorySize = Clay_MinMemorySize();
    arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
    Clay_Initialize(arena, (Clay_Dimensions) { 1920, 1080 },(Clay_ErrorHandler) { HandleClayErrors });
    return 0;
}

static inline void* get_pixel(uint32_t x, uint32_t y, uint8_t* surface){
    return &surface[y * (4 * 1920) + (x) * 4];
}

static inline void render_rectangle(uint8_t* surface, Clay_BoundingBox bounds, Clay_Color color){
    for(uint32_t x = bounds.x; x < bounds.width; x++){
        for (uint32_t y = bounds.y; y < bounds.height; y++) {
            void* s = get_pixel(x, y, surface);
            *(uint32_t*)s = 255;
        }
    }
}

static inline uint32_t render_frame2(uint8_t* surface, uint32_t width, uint32_t height){
    // if(surface[0] <= 255){
    //     memset(surface, surface[0]+1, (width * height) * 4);
    //     //return 1;
    // }

    // if(surface[0] >= 255){
    //     memset(surface, 0, (width * height) * 4);
    //     //return 1;
    // }
    Clay_BeginLayout();

    CLAY(CLAY_ID("OuterContainer"), { .layout = { .sizing = {CLAY_SIZING_FIXED(500), CLAY_SIZING_FIXED(500)}, .padding = CLAY_PADDING_ALL(16), .childGap = 16 }, .backgroundColor = {250,250,255,255} }) {
        CLAY(CLAY_ID("SideBar"), {
            .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = { .width = CLAY_SIZING_FIXED(300), .height = CLAY_SIZING_GROW(0) }, .padding = CLAY_PADDING_ALL(16), .childGap = 16 },
            .backgroundColor = (Clay_Color) { 224, 215, 210, 255}
        }) {
            CLAY(CLAY_ID("ProfilePictureOuter"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) }, .padding = CLAY_PADDING_ALL(16), .childGap = 16, .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } }, .backgroundColor = (Clay_Color) {168, 66, 28, 255} }) {
                //CLAY(CLAY_ID("ProfilePicture"), { .layout = { .sizing = { .width = CLAY_SIZING_FIXED(60), .height = CLAY_SIZING_FIXED(60) }}, .image = { .imageData = &profilePicture } }) {}
                //CLAY_TEXT(CLAY_STRING("Clay - UI Library"), { .fontSize = 24, .textColor = {255, 255, 255, 255} });
            }

            CLAY(CLAY_ID("MainContent"), { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } }, .backgroundColor = (Clay_Color) {224, 215, 210, 255} }) {}
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();

    for (int i = 0; i < renderCommands.length; i++) {
        Clay_RenderCommand* renderCommand = &renderCommands.internalArray[i];

        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                render_rectangle(surface, renderCommand->boundingBox, renderCommand->renderData.rectangle.backgroundColor);
            }
        }
    }
    return 0;
}

static inline uint32_t cleanup_renderer(){
    return 0;
}
