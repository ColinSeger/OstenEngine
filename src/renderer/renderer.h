#pragma once
#include <string.h>
#include <stdint.h>

static inline uint32_t setup_renderer(){
    return 0;
}

static inline uint32_t render_frame2(uint8_t* surface, uint32_t width, uint32_t height){
    if(surface[0] <= 255){
        memset(surface, surface[0]+1, (width * height) * 4);
        return 1;
    }

    if(surface[0] >= 255){
        memset(surface, 0, (width * height) * 4);
        return 1;
    }
    // for(uint32_t y = 0; y < height; y ++){
    //     for(uint32_t x = 0; x < width; x ++){
    //         surface[width * height] = x;
    //     }
    // }
    return 0;
}

static inline uint32_t cleanup_renderer(){
    return 0;
}
