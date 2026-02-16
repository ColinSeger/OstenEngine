#pragma once
#include <cstdint>

struct QuadTree{
    union{
        uint16_t quad_index[4];
        uint16_t positions[8];
    };
};

static inline uint16_t insert_location(){
    return 0;
}
