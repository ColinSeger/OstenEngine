#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include "../../../external/math_3d.h"
#include "../device/vulkan/device.cpp"
#include "../../debugger/debugger.h"

typedef struct{
    uint32_t vertex_index;
    uint32_t texture_index;
    uint32_t normal_index;
} Indices;

static constexpr bool select_mode(char* char_to_check)//This is ass
{
    if (*char_to_check == 'v') {
        char_to_check++;
        if(*char_to_check == ' ' || *char_to_check == 't' || *char_to_check == 'n'){
            return true;
        }
        return false;
    }
    if(*char_to_check == 'f'){
        return true;
    }
    return false;
}

static inline vec3_t parse_vertex(const std::string& line, const uint16_t start_index){
    vec3_t result {};
    size_t cord_index = start_index;

    result.x = parse_float(&line[cord_index++], cord_index);
    result.y = parse_float(&line[cord_index++], cord_index);
    result.z = parse_float(&line[cord_index++], cord_index);

    return result;
}

static inline vec2_t parse_uv(const std::string& line, const uint16_t start_index){
    vec2_t result{};
    size_t cord_index = start_index;

    result.x = parse_float(&line[cord_index++], cord_index);
    result.y = parse_float(&line[cord_index++], cord_index);

    return result;
}

static inline size_t parse_indicie(const std::string& line, const uint16_t start_index, HeapStack* heap_stack){
    uint32_t index_index = 0;
    uint8_t temp = 0;
    uint32_t indicies[12]{};
    size_t memory_index = arena_alloc_memory(heap_stack, sizeof(uint32_t) * 9);
    for (size_t i = start_index; i < line.length(); i++){
        uint32_t value = parse_to_uint32(&line[i+1], &i);

        if(line[i] == '/' || line[i] == ' '){
            indicies[index_index] = value;
            index_index++;
            if(line[i] == '/') temp++;
        }
    }
    uint32_t* values = (uint32_t*)get_at_index(heap_stack, memory_index);
    if(index_index < 9){
        if(temp < 5){
            values[0] = indicies[0];
            values[1] = indicies[1];
            values[2] = 0;

            values[3] = indicies[2];
            values[4] = indicies[3];
            values[5] = 0;

            values[6] = indicies[4];
            values[7] = indicies[5];
            values[8] = 0;
        }else{
            values[0] = indicies[0];
            values[1] = 0;
            values[2] = indicies[1];

            values[3] = indicies[2];
            values[4] = 0;
            values[5] = indicies[3];

            values[6] = indicies[4];
            values[7] = 0;
            values[8] = indicies[5];
        }

        return memory_index;
    }
    else if(index_index > 9){
        size_t memory_index2 = arena_alloc_memory(heap_stack, sizeof(uint32_t) * 9);
        uint32_t* values2 = (uint32_t*)get_at_index(heap_stack, memory_index2);
        values2[0] = indicies[0];
        values2[1] = indicies[1];
        values2[2] = indicies[2];

        memcpy(&values2[3], &indicies[6], sizeof(uint32_t) * 6);
    }
    memcpy(values, indicies, sizeof(uint32_t) * 9);
    return memory_index;
}

static inline size_t load_obj_v2(const char* path_of_obj, VertexArray& model_vertices, Uint32Array& model_indicies, HeapStack* heap_stack){
    Debug::profile_time_start();
    std::ifstream file_stream(path_of_obj, std::ios_base::in);

    if(!file_stream.is_open()){
        model_vertices.values = nullptr;
        model_vertices.amount = 0;
        model_indicies.values = nullptr;
        model_indicies.amount = 0;
        Debug::log((char*)"Failed to load model");
        return heap_stack->capacity;
    }
    size_t mem_index = heap_stack->index;

    size_t texture_index = 0;
    size_t normals_index = 0;

    size_t vertex_end = mem_index;
    size_t indicie_start = 0;
    std::string line;
    while(getline(file_stream, line)){
        if(line[0] == 'v' && line[1] == ' '){
            size_t index = arena_alloc_memory(heap_stack, sizeof(Vertex));

            Vertex* write_to = (Vertex*)get_at_index(heap_stack, index);
            write_to->position = parse_vertex(line, 2);
            vertex_end = index + sizeof(Vertex);
        }
        else if(line[0] == 'v' && line[1] == 't'){
            size_t index = arena_alloc_memory(heap_stack, sizeof(vec2_t));
            vec2_t* write_to = (vec2_t*)get_at_index(heap_stack, index);
            vec2_t t = parse_uv(line, 3);
            write_to->x = t.x;
            write_to->y = t.y;
            if(texture_index == 0) texture_index = index;
        }
        else if(line[0] == 'v' && line[1] == 'n'){
            size_t index = arena_alloc_memory(heap_stack, sizeof(vec3_t));
            vec3_t* write_to = (vec3_t*)get_at_index(heap_stack, index);
            vec3_t t = parse_vertex(line, 3);
            write_to->x = t.x;
            write_to->y = t.y;
            write_to->z = t.z;
            if(normals_index == 0) normals_index = index;
        }
        else if(line[0] == 'f' && line[1] == ' '){
            size_t mem_i = parse_indicie(line, 1, heap_stack);
            if(indicie_start == 0)  indicie_start = mem_i;
        }
    }
    file_stream.close();

    size_t vertex_bytes = (vertex_end - mem_index);
    model_vertices.amount = (vertex_bytes / sizeof(Vertex));
    model_vertices.values = (Vertex*)get_at_index(heap_stack, mem_index);

    size_t index = 0;
    size_t index_end = 0;
    size_t indicie_index = (heap_stack->index - indicie_start) / (sizeof(uint32_t)*3);
    for (size_t i = 0; indicie_index > i; i++) {
        size_t index_of = indicie_start + (sizeof(uint32_t) * 3) * i;
        size_t indicie = *((uint32_t*)get_at_index(heap_stack, index_of)) -1;
        size_t texture_uv_index = *((uint32_t*)get_at_index(heap_stack, index_of + sizeof(uint32_t))) -1;
        size_t vertex_normal_index = *((uint32_t*)get_at_index(heap_stack, index_of + sizeof(uint32_t)+ sizeof(uint32_t))) -1;


        if(!index){
            index = arena_alloc_memory(heap_stack, sizeof(uint32_t));
        }else{
            index_end = arena_alloc_memory(heap_stack, sizeof(uint32_t));
        }

        uint32_t* indicies = (uint32_t*)get_at_index(heap_stack, index + (sizeof(uint32_t) * i));
        *indicies = indicie;

        if(normals_index){
            model_vertices.values[indicie].normals = *((vec3_t*)get_at_index(heap_stack, (vertex_normal_index * sizeof(vec3_t))+ normals_index));
        }else{
            model_vertices.values[indicie].normals = {0,1,0};
        }

        model_vertices.values[indicie].texture_cord.x = ((vec2_t*)get_at_index(heap_stack, (texture_uv_index * sizeof(vec2_t))+ texture_index))->x;
        model_vertices.values[indicie].texture_cord.y = 1.f - ((vec2_t*)get_at_index(heap_stack, (texture_uv_index * sizeof(vec2_t))+ texture_index))->y;
    }

    size_t allocation_index = arena_alloc_memory(heap_stack, indicie_index * sizeof(uint32_t));

    model_indicies.amount = indicie_index;
    model_indicies.values = (uint32_t*)get_at_index(heap_stack, allocation_index);

    memcpy(model_indicies.values, get_at_index(heap_stack, index), (index_end - index) + sizeof(uint32_t));

    Debug::profile_time_end();
    return mem_index;
}
