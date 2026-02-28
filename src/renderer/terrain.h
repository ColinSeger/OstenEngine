#pragma once
#include <stdint.h>
#include "device/vulkan/device.h"

struct Terrain{
    Vertex* vertexes;
    uint32_t* indexes;
    uint32_t vertex_amount;
    uint32_t index_amount;
};

static inline void create_terrain(Terrain* terrain){

}
