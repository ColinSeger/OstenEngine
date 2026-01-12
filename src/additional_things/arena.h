#pragma once
#include <string.h>
#include "../platform.h"
#include "../debugger/debugger.h"

struct MemArena{
    unsigned long long capacity;
    unsigned long long index;
    unsigned char* data;
    void* operator[](const unsigned long long index) const {
        if(index > capacity || this->index < index) throw;
        return &data[index];
    }
};

inline MemArena init_mem_arena(unsigned long long capacity){
    return MemArena{
        capacity,
        0,
        (unsigned char*) platform_alloc_memory(capacity* sizeof(unsigned char))
    };
}

inline bool arena_expand(MemArena& arena, unsigned long long passed_in){
    Debug::log((char*)"Arena Expanded, Consider increasing base size");
    unsigned long long new_size = arena.capacity * 2;
    while (arena.index + passed_in > new_size) {
        new_size*=2;
    }

    unsigned char* new_data = (unsigned char*)platform_alloc_memory(new_size);
    unsigned char* old_data = arena.data;
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
inline void free_arena(MemArena& arena, unsigned long long index){
    if(index > arena.capacity) throw;
    if(index > arena.index) return;
    arena.index = index;
}

inline void destroy_arena(MemArena& arena){
    arena.capacity = 0;
    arena.index = 0;
    platform_free_memory(arena.data, arena.capacity);
}
