#pragma once
#include <stdint.h>
#include <stdio.h>
#include "../external/osten_math.h"
#include "platform.h"
#include "additional_things/arena.h"
#include "renderer/renderer.h"

#define KB 1024
#define MB  (KB * 1024)
#define GB  (MB * 1024)

typedef struct OstenEngine{
    MemArena mem_arena;
} OstenEngine;

void update_render_game(OstenEngine* engine, OstenWindow* window, vec2 mouse_cords){
    render_frame(window, mouse_cords);
}
