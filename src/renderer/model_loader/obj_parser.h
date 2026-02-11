#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
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
    float result[3]{};
    uint8_t cord_index = 0;
    for (size_t i = start_index; i < line.length(); i++){
        if(line[i] == ' '){
            result[cord_index] = parse_float(&line[i+1], i);

            cord_index ++;
            if(cord_index >= 3)break;
        }
    }
    return {result[0], result[1], result[2]};
}

static inline vec2_t parse_uv(const std::string& line, const uint16_t start_index)
{
    float result[2]{};
    uint8_t cord_index = 0;
    for (size_t i = start_index; i < line.length(); i++){
        if(line[i] == ' '){
            result[cord_index] = parse_float(&line[i+1], i);

            cord_index ++;
            if(cord_index >= 2)break;
        }
    }
    return {result[0], result[1]};
}

static inline size_t parse_indicie(const std::string& line, const uint16_t start_index, HeapStack& heap_stack){
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
    uint32_t* values = (uint32_t*)heap_stack[memory_index];
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
        uint32_t* values2 = (uint32_t*)heap_stack[memory_index2];
        values2[0] = indicies[0];
        values2[1] = indicies[1];
        values2[2] = indicies[2];

        memcpy(&values2[3], &indicies[6], sizeof(uint32_t) * 6);
    }
    memcpy(values, indicies, sizeof(uint32_t) * 9);
    return memory_index;
}

static inline size_t load_obj_v2(const char* path_of_obj, VertexArray& model_vertices, Uint32Array& model_indicies, HeapStack& heap_stack){
    Debug::profile_time_start();
    std::ifstream file_stream(path_of_obj, std::ios_base::in);

    if(!file_stream.is_open()){
        model_vertices.values = nullptr;
        model_vertices.amount = 0;
        model_indicies.values = nullptr;
        model_indicies.amount = 0;
        Debug::log((char*)"Failed to load model");
        return heap_stack.capacity;
    }

    // size_t file_size = file_stream.tellg();
    // file_stream.seekg(0);
    // //Funky idea where I overdide my old memory with the actual data to reduce memory allocation
    //
    size_t mem_index = heap_stack.index;


    std::vector<vec3_t> normals;
    std::vector<vec2_t> uv;

    size_t vertex_end = mem_index;
    size_t indicie_start = 0;
    std::string line;
    while(getline(file_stream, line)){
        if(line[0] == 'v' && line[1] == ' '){
            size_t index = arena_alloc_memory(heap_stack, sizeof(Vertex));

            Vertex* write_to = (Vertex*)heap_stack[index];
            write_to->position = parse_vertex(line, 1);
            vertex_end = index + sizeof(Vertex);
        }
        else if(line[0] == 'v' && line[1] == 'n'){
            normals.emplace_back(parse_vertex(line, 1));
        }
        else if(line[0] == 'v' && line[1] == 't'){
            uv.emplace_back(parse_uv(line, 1));
        }
        else if(line[0] == 'f' && line[1] == ' '){
            size_t mem_i = parse_indicie(line, 1, heap_stack);
            if(indicie_start == 0)  indicie_start = mem_i;
        }
    }

    size_t vertex_bytes = (vertex_end - mem_index);
    model_vertices.amount = (vertex_bytes / sizeof(Vertex));
    model_vertices.values = (Vertex*)heap_stack[mem_index];

    std::vector<uint32_t> temp;
    size_t indicie_index = (heap_stack.index - indicie_start) / (sizeof(uint32_t)*3);
    for (size_t i = 0; indicie_index > i; i++) {
        size_t index_of = indicie_start + (sizeof(uint32_t) * 3) * i;
        size_t indicie = *((uint32_t*)heap_stack[index_of]) -1;
        size_t texture_uv_index = *((uint32_t*)heap_stack[index_of + sizeof(uint32_t)]) -1;
        size_t vertex_normal_index = *((uint32_t*)heap_stack[index_of + sizeof(uint32_t)+ sizeof(uint32_t)]) -1;

        temp.push_back(indicie);

        if(!normals.empty()){
            model_vertices.values[indicie].normals = normals[vertex_normal_index];
        }else{
            model_vertices.values[indicie].normals = {0,1,0};
        }

        model_vertices.values[indicie].texture_cord.x = uv[texture_uv_index].x;
        model_vertices.values[indicie].texture_cord.y = 1.f - uv[texture_uv_index].y;
    }

    model_indicies.amount = indicie_index;
    free_arena(heap_stack, indicie_start);//Temp
    size_t allocation_size = model_indicies.amount * sizeof(uint32_t);
    size_t allocation_index = arena_alloc_memory(heap_stack, allocation_size);

    model_indicies.values = (uint32_t*)heap_stack[allocation_index];

    size_t range = sizeof(uint32_t) * temp.size()-1;
    memcpy(model_indicies.values, temp.data(), range);

    file_stream.close();

    Debug::profile_time_end();
    return mem_index;
}

/*
uint16_t* lin2 = (uint16_t*)line.data();
size_t index = mem_index;

Vertex* write_to = (Vertex*)heap_stack[index];
switch (*lin2) {
    case 8310://Vertex
        index = arena_alloc_memory(heap_stack, sizeof(Vertex));
        write_to->position = parse_vertex(line, 1);
        vertex_end = index + sizeof(Vertex);
    break;
    case 29814://UV
        uv.emplace_back(parse_uv(line, 1));
    break;
    case 28278://Normal
        normals.emplace_back(parse_vertex(line, 1));
    break;
    case 8294://Index
        size_t mem_i = parse_indicie(line, 1, heap_stack);
        if(indicie_start == 0)  indicie_start = mem_i;
    break;
}
 *
 */
