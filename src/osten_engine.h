#pragma once
#include <stdint.h>
#include "../external/osten_math.h"
#include "arena.h"
#include "renderer/renderer.h"


typedef struct OstenEngine{
    MemArena mem_arena;
} OstenEngine;

static OstenEngine osten_engine;

static inline void init_engine(OstenWindow *engine_window, OstenEngine* engine){
    init_mem_arena(&osten_engine.mem_arena, 256 * MEGABYTES);
    *engine = osten_engine;
}

typedef struct EngineData {
    OstenEngine* engine;
    OstenWindow* window;
    vec2 mouse_cords;
} EngineData;

static inline void* update_render_game(void* test){
    EngineData t = *(EngineData*)test;
    //void* free = arena_alloc_memory(&t.engine->mem_arena, 0);
    render_frame(t.window, t.mouse_cords, &t.engine->mem_arena);
    //pop_arena(&t.engine->mem_arena, (u8*)free);
    return 0;
}
