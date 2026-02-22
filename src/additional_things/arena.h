#pragma once
#include <string.h>
#include <stdbool.h>
#include "../platform.h"

struct HeapStack{
    unsigned long long capacity;
    unsigned long long index;
    unsigned char* data;
};

inline void* get_at_index(struct HeapStack* heap_stack, const unsigned long long index){
    if(index > heap_stack->capacity || heap_stack->index < index) return 0;
    return &heap_stack->data[index];
}

inline bool init_mem_arena(struct HeapStack* heap_stack, unsigned long long capacity){
    heap_stack->capacity = capacity;
    heap_stack->index = 0;
    heap_stack->data = (unsigned char*) platform_alloc_memory(capacity* sizeof(unsigned char));

    return true;
}

inline bool arena_expand(struct HeapStack* heap_stack, unsigned long long passed_in){
    //Debug::log((char*)"Arena Expanded, Consider increasing base size");
    unsigned long long new_size = heap_stack->capacity * 2;
    while (heap_stack->index + passed_in > new_size) {
        new_size*=2;
    }

    unsigned char* new_data = (unsigned char*)platform_alloc_memory(new_size);
    unsigned char* old_data = heap_stack->data;
    memcpy(new_data, old_data, heap_stack->capacity);
    platform_free_memory(old_data, heap_stack->capacity);
    heap_stack->capacity = new_size;
    heap_stack->data = new_data;
    return true;
}

//Reserves the requested size and return it's index to you
inline unsigned long long arena_alloc_memory(struct HeapStack* arena, unsigned long long size){
    while(arena->capacity < size + arena->index) arena_expand(arena, size);
    arena->index += size;
    return arena->index - size;
}

//This will free all values after index
inline void free_arena(struct HeapStack* arena, unsigned long long index){
    if(index > arena->capacity) return;
    if(index > arena->index) return;
    arena->index = index;
}

inline void destroy_arena(struct HeapStack* arena){
    arena->capacity = 0;
    arena->index = 0;
    platform_free_memory(arena->data, arena->capacity);
}
