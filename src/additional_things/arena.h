#pragma once
#include <cstdint>
#include <cstdlib>
#include "../platform.h"

struct MemArena{
    unsigned long long capacity;
    unsigned long long index;
    uint8_t* data;
    void* operator[](const unsigned long long index) const {
        if(index > capacity) throw;
        return &data[index];
    }
};

inline MemArena init_mem_arena(unsigned long long capacity)
{
    return MemArena{
        capacity,
        0,
        (uint8_t*) platform_alloc_memory(capacity* sizeof(uint8_t))
    };
}

//Reserves the requested size and return it's index to you
inline unsigned long long arena_alloc_memory(MemArena& arena, unsigned long long size){
    if(arena.capacity < size + arena.index) throw;
    arena.index += size;
    return arena.index - size;
}

//This will free all values after index
inline void free_arena(MemArena& arena, unsigned long long index)
{
    if(index > arena.capacity) throw;
    if(index > arena.index) return;
    arena.index = index;
}

inline void destroy_arena(MemArena& arena){
    arena.capacity = 0;
    arena.index = 0;
    free(arena.data);
}

/*
inline void* request_arena_memory(MemArena& arena,unsigned long long index){
    if(index > arena.capacity) throw;
    return &arena.data[index];
} */
