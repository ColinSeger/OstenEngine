#pragma once
#include <stdint.h>
#include "../platform.h"

/**
    This is a memory arena
*/
typedef struct{
    uint64_t capacity;
    uint64_t index;
    uint8_t* data;
} MemArena;

// static inline void* get_at_index(MemArena* heap_stack, uint64_t index){
//     if(index > heap_stack->capacity || heap_stack->index < index) return 0;
//     return &heap_stack->data[index];
// }

static inline uint32_t init_mem_arena(MemArena* mem_arena, uint64_t capacity){
    *mem_arena = (MemArena){
        .capacity = capacity,
        .index = 0,
        .data = (uint8_t*)platform_alloc_memory(capacity)
    };
    return 1;
}

static inline uint32_t arena_expand(MemArena* mem_arena, uint64_t passed_in){
    //Debug::log((char*)"Arena Expanded, Consider increasing base size");
    uint64_t new_size = mem_arena->capacity * 2;
    while (mem_arena->index + passed_in > new_size) {
        new_size*=2;
    }

    uint8_t* new_data = (uint8_t*)platform_alloc_memory(new_size);
    uint8_t* old_data = mem_arena->data;
    for(uint64_t i = 0; i < mem_arena->capacity; i++){
        new_data[i] = old_data[i];
    }
    //memcpy(new_data, old_data, mem_arena->capacity);
    platform_free_memory(old_data, mem_arena->capacity);
    mem_arena->capacity = new_size;
    mem_arena->data = new_data;
    return 1;
}

//Reserves the requested size and return it's index to you
static inline uint8_t* arena_alloc_memory(MemArena* arena, uint64_t size){
    arena->index += size;
    return &arena->data[arena->index - size];
}

//This will free all values after index
static inline uint32_t pop_arena(MemArena* arena, uint8_t* index){

    arena->index = index - arena->data;
    // if(index > arena->capacity) return 0;
    // if(index > arena->index) return 0;
    // arena->index = index;
    return 1;
}

static inline uint32_t destroy_arena(MemArena* arena){
    arena->capacity = 0;
    arena->index = 0;
    platform_free_memory(arena->data, arena->capacity);
    return 1;
}
