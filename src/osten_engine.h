#pragma once
#include <stdint.h>
//#include "../external/RGFW.h"
#include "../external/math_3d.h"
#include "platform.h"
#include "additional_things/arena.h"
#include "renderer/renderer.h"

#define KB 1024
#define MB  (KB * 1024)
#define GB  (MB * 1024)


typedef struct OstenEngine{

    MemArena mem_arena;

} OstenEngine;

//static Timer start_time2;
//static Timer last_tick;

static inline void create_osten_engine(const uint32_t width, const uint32_t height, const char* application_name, OstenEngine* engine){
    init_mem_arena(&engine->mem_arena, 256 * MB);

    setup_renderer(&engine->mem_arena, width, height);
}

static inline void update_render_game(OstenEngine* engine, OstenWindow* window, vec2_t mouse_cords){

    render_frame(window, mouse_cords);
}

static inline void cleanup(OstenEngine* engine){
    destroy_arena(&engine->mem_arena);
}
