#pragma once
#include <cstdint>
#include <fstream>
// #include <string>
#include <vulkan/vulkan.h>
#include <vector>
#include "../device/vulkan/device.cpp"
#include "../../debugger/debugger.cpp"

typedef struct
{
    uint32_t index_amount;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;//TODO Look into how to merge into vertex buffer
    VkDeviceMemory index_buffer_memory;
} Model;

enum class LoadMode
{
    OBJ,
    BIN
};

namespace ModelLoader
{
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

    static void parse_obj(const char* path_of_obj, VertexArray& model_vertices, Uint32Array& model_indicies)
    {
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
            float texture_cord[2];
        };
        ValueToAdd value_to_add{};

        std::vector<Vertex> vertex;
        std::vector<Indices> indicies;
        std::vector<TextureCord> texture_cords;
        vertex.reserve(file_size/40);
        texture_cords.reserve(file_size/60);
        indicies.reserve(file_size/30);

        size_t index = 0;
        uint8_t char_index = 0;

        next_valid(file, &index, file_size);

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

        for (size_t i = index; i < file_size; i++){
            if (file[i] == '\n') {
                char_index = 0;
                TextureCord cord {};
                cord.x = value_to_add.texture_cord[0];
                cord.y = 1.f - value_to_add.texture_cord[1];
                texture_cords.emplace_back(cord);
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

        for (size_t i = index; i < file_size; i++){
            if (file[i] == '\n') {
                char_index = 0;
                if(file[i+1] != 'v' || file[i+2] != 'n'){
                    index = i;
                    break;
                }
            }
            if(file[i] == ' '){//TODO
                // vertex_to_add[char_index] = atof(&file[i+1]);
                // char_index ++;
            }
        }

        next_valid(file, &index, file_size);

        uint32_t value_index = 0;
        Indices triangle_indexes {};

        Debug::profile_time_start();

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

        Debug::profile_time_end();

        model_indicies.values = (uint32_t*)malloc(indicies.size() * sizeof(uint32_t));
        model_indicies.amount = 0;
        for (size_t i = 0; i < indicies.size(); i++)
        {
            uint32_t val = indicies[i].vertex_index;
            uint32_t bal = indicies[i].texture_index;
            if(val > 0 && bal > 0){
                vertex[indicies[i].vertex_index-1].texture_cord = texture_cords[indicies[i].texture_index-1];
                model_indicies.values[model_indicies.amount] = indicies[i].vertex_index-1;
                model_indicies.amount++;
            }
        }
        model_vertices.values = vertex.data();
        model_vertices.amount = vertex.size();

        free(file);
    }

    static void serialize(const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
    {
        std::ofstream file(filename, std::ios::binary);

        if(!file.is_open()){
            abort();
        }
        uint32_t index_start = vertices.size() * sizeof(Vertex);

        file.write(reinterpret_cast<const char*>(&index_start), sizeof(uint32_t));

        file.write(reinterpret_cast<char*>(vertices.data()), index_start);

        file.write(reinterpret_cast<char*>(indices.data()),  indices.size() * sizeof(uint32_t));

        /*
        for(Vertex vertex : vertices){
            file.write(reinterpret_cast<const char*>(&vertex), sizeof(Vertex));
        }

        for(uint32_t index : indices){
            file.write(reinterpret_cast<const char*>(&index), sizeof(uint32_t));
        }

         */
        file.close();
    }

    //This Returns a char* you need to free after use
    static char* de_serialize(const char* filename, VertexArray& vertices, Uint32Array& indices)
    {
        Debug::profile_time_start();
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if(!file.is_open()){
            Debug::log((char*)"There was a issue parsing this model");
            return nullptr;
        }

        //Find out where the file ends
        size_t file_size = file.tellg();
        file.seekg(0);
        char* buffer = (char*)malloc(file_size);

        file.read(buffer, file_size);
        file.close();
        //195528 debug asset

        //Figures out where the vertexes starts/stops
        uint32_t index_start = *reinterpret_cast<uint32_t*>(buffer);

        //Sets where to start values of the model
        vertices.values = reinterpret_cast<Vertex*>(buffer + sizeof(uint32_t));
        indices.values = reinterpret_cast<uint32_t*>(buffer+ sizeof(uint32_t) + index_start);

        //Sets amount the array contains
        vertices.amount = index_start / sizeof(Vertex);
        indices.amount = (file_size - index_start) / sizeof(uint32_t);

        Debug::profile_time_end();
        return buffer;
    }

    Model load_model(Device& device, VkCommandPool command_pool, const char* file_name, LoadMode load_mode)
    {
        Model model{};
        char* file_to_free;//Temp solution for speed test
        VertexArray vertices;
        Uint32Array indices;

        if(load_mode == LoadMode::OBJ){
            parse_obj(file_name, vertices, indices);
        }else if(load_mode == LoadMode::BIN){
            file_to_free = de_serialize(file_name, vertices, indices);
        }

        model.index_amount = indices.amount;
        CommandBuffer::create_vertex_buffer(device, vertices, model.vertex_buffer, model.vertex_buffer_memory, command_pool);
        CommandBuffer::create_index_buffer(device, indices, model.index_buffer, model.index_buffer_memory, command_pool);

        free(file_to_free);

        return model;
    }
}/**/
