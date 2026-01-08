#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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

inline bool arena_expand(MemArena& arena, size_t passed_in)
{
    printf("Resized Arena \n");
    unsigned long long new_size = arena.capacity * 2;
    while (arena.index + passed_in > new_size) {
        new_size*=2;
    }

    uint8_t* new_data = (uint8_t*)platform_alloc_memory(new_size);
    uint8_t* old_data = arena.data;
    memcpy(new_data, old_data, arena.capacity);
    platform_free_memory(old_data, arena.capacity);
    arena.capacity = new_size;
    arena.data = new_data;
    return true;
}

//Reserves the requested size and return it's index to you
inline unsigned long long arena_alloc_memory(MemArena& arena, unsigned long long size){
    while(arena.capacity < size + arena.index) arena_expand(arena, size);
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
