#pragma once
#include <stdint.h>
#include "../external/osten_math.h"
#include "additional_things/arena.h"
#include "renderer/renderer.h"


typedef struct OstenEngine{
    MemArena mem_arena;
} OstenEngine;

static OstenEngine osten_engine;

void init_engine(OstenWindow *engine_window, OstenEngine* engine){
    init_mem_arena(&osten_engine.mem_arena, 256 * MEGABYTES);
    debug_init_clay(engine_window, &osten_engine.mem_arena);
    *engine = osten_engine;
}

void update_render_game(OstenEngine* engine, OstenWindow* window, vec2 mouse_cords){
    render_frame(window, mouse_cords);
}
