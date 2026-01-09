#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
#include "../device/vulkan/device.cpp"
#include "../../debugger/debugger.cpp"

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

static constexpr void next_valid(char* file, size_t* current_value, size_t max_value){
    for (size_t i = *current_value; i < max_value; i++){
        if (select_mode(&file[i])){
            break;
        }
        *current_value = i+1;
    }
}

static inline uint32_t parse_to_uint32(const char* start, size_t* index_jump){
    uint32_t result = 0;
    while (*start >= '0' && *start <= '9') {
        index_jump++;
        result = result * 10 + (*start - '0');
        ++start;
    }
    return result;
}

static inline Vertex parse_to_vertex(const char* file, size_t index, size_t file_size)
{
    uint8_t char_index = 0;

    Vertex vertex{};

    //Vertexes
    for (size_t i = index; i < file_size; i++){
        if (file[i] == '\n') {
            char_index = 0;
            return vertex;
            if(file[i+1] != 'v' || file[i+2] != ' '){
                index = i;
                break;
            }
        }
        if(file[i] == ' '){
            //vertex[char_index] = atof(&file[i+1]);
            char_index ++;
        }
    }
    return vertex;
}

static inline void parse_obj(const char* path_of_obj, VertexArray& model_vertices, Uint32Array& model_indicies, MemArena& memory_arena)
{
    Debug::profile_time_start();
    std::ifstream file_stream(path_of_obj, std::ios_base::in | std::ios_base::ate);

    if(!file_stream.is_open())
    {
        model_vertices.values = nullptr;
        model_vertices.amount = 0;
        model_indicies.values = nullptr;
        model_indicies.amount = 0;
        Debug::log((char*)"Failed to load model");
        return;
    }

    size_t file_size = file_stream.tellg();
    file_stream.seekg(0);
    char* file = (char*)malloc(sizeof(char) * file_size);

    file_stream.read(file, file_size);
    file_stream.close();


    union ValueToAdd {
        float vertex_to_add[3];
        float indicies_to_add[3];
        float normal_cords[3];
        float texture_cord[2];
    };
    ValueToAdd value_to_add{};

    std::vector<Vertex> vertex;
    std::vector<Indices> indicies;
    std::vector<TextureCord> texture_cords;
    std::vector<vec3_t> vertex_normals;
    vertex.reserve(file_size/40);
    texture_cords.reserve(file_size/60);
    indicies.reserve(file_size/30);

    size_t index = 0;
    uint8_t char_index = 0;

    next_valid(file, &index, file_size);

    //Vertexes
    for (size_t i = index; i < file_size; i++){
        if (file[i] == '\n') {
            char_index = 0;
            vertex.emplace_back(Vertex{{value_to_add.vertex_to_add[0], value_to_add.vertex_to_add[1], value_to_add.vertex_to_add[2]}, {0, 0, 0}, {0, 0}});
            if(file[i+1] != 'v' || file[i+2] != ' '){
                index = i;
                break;
            }
        }
        if(file[i] == ' '){
            value_to_add.vertex_to_add[char_index] = atof(&file[i+1]);
            char_index ++;
        }
    }

    next_valid(file, &index, file_size);

    //TextureCords
    for (size_t i = index; i < file_size; i++){
        if (file[i] == '\n') {
            char_index = 0;
            TextureCord cord {};
            cord.x = value_to_add.texture_cord[0];
            cord.y = 1.f - value_to_add.texture_cord[1];
            texture_cords.push_back(cord);
            if(file[i+1] != 'v' || file[i+2] != 't'){
                index = i;
                break;
            }
        }
        if(file[i] == ' '){
            value_to_add.texture_cord[char_index] = atof(&file[i+1]);
            char_index ++;
        }
    }

    next_valid(file, &index, file_size);

    //Normals
    for (size_t i = index; i < file_size; i++){
        if (file[i] == '\n') {
            char_index = 0;
            vertex_normals.push_back({value_to_add.normal_cords[0], value_to_add.normal_cords[1], value_to_add.normal_cords[2]});
            if(file[i+1] != 'v' || file[i+2] != 'n'){
                index = i;
                break;
            }
        }
        if(file[i] == ' '){//TODO
            value_to_add.normal_cords[char_index] = atof(&file[i+1]);
            char_index ++;
        }
    }

    next_valid(file, &index, file_size);

    uint32_t value_index = 0;
    Indices triangle_indexes {};


    for (size_t i = index; i < file_size; i++){
        if (file[i] == '\n') {
            for (size_t t = i; t < file_size; t++)
            {
                if(file[t] == 'f'){
                    indicies.emplace_back(triangle_indexes);
                    i = t;
                    value_index = 0;
                    break;
                }
            }
        }
        else if(file[i] == '/'){
            uint32_t value = parse_to_uint32(&file[i+1], &i);
            if(value_index <= 1){
                triangle_indexes.texture_index = value;
                value_index++;
            }else if(value_index > 1){
                triangle_indexes.normal_index = value;
            }
            continue;
        }
        else if(file[i] == ' '){
            uint32_t value = parse_to_uint32(&file[i+1], &i);
            if(value_index <= 0){
                triangle_indexes.vertex_index = value;
                value_index++;
            }else{
                indicies.emplace_back(triangle_indexes);
                value_index = 1;
                triangle_indexes.vertex_index = value;
            }
            continue;
        }
    }


    model_indicies.values = (uint32_t*)malloc(indicies.size() * sizeof(uint32_t));
    model_indicies.amount = 0;
    for (size_t i = 0; i < indicies.size(); i++)
    {
        uint32_t vertex_index = indicies[i].vertex_index-1;
        uint32_t texture_index = indicies[i].texture_index-1;
        uint32_t normal_index = indicies[i].normal_index-1;
        if(vertex_index >= 0 && texture_index >= 0){
            vertex[vertex_index].texture_cord = texture_cords[texture_index];
            vertex[vertex_index].normals = vertex_normals[normal_index];
            model_indicies.values[model_indicies.amount] = vertex_index;
            model_indicies.amount++;
        }
    }

    model_vertices.values = (Vertex*)malloc(sizeof(Vertex) * vertex.size());
    memcpy(model_vertices.values, vertex.data(), sizeof(Vertex) * vertex.size());
    model_vertices.amount = vertex.size();

    free(file);
    Debug::profile_time_end();
}
