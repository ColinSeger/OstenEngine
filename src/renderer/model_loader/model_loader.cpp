// #include "model_loader.h"
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vulkan/vulkan.h>
#include <vector>
#include "../device/vulkan/device.cpp"

enum class ObjMode : uint8_t
{
    Vertex,
    Normal,
    TextureCord,
    Face,
    None,
    Comment
};

struct Model
{
    uint32_t index_amount = 0;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;//TODO Look into how to merge into vertex buffer
    VkDeviceMemory index_buffer_memory;
};


namespace ModelLoader
{
    const char valid_chars[14] = "0123456789.-/";
    struct Indices{
        uint32_t vertex_index;
        uint32_t texture_index;
        uint32_t normal_index;
    };

    static bool is_valid_char(char c)
    {
        for(char valid : valid_chars)
        {
            if(c == valid) return true;
        }
        return false;
    }

    static inline bool select_mode(char* char_to_check)//This is ass
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

    static inline void next_valid(char* file, size_t* current_value, size_t max_value){
        for (size_t i = *current_value; i < max_value; i++){
            if (select_mode(&file[i])){
                break;
            }
            *current_value = i+1;
        }
    }

    static void parse_obj(const char* path_of_obj, std::vector<Vertex>& model_vertices, std::vector<uint32_t>& model_indicies)
    {
        std::ifstream file_stream(path_of_obj, std::ios_base::in | std::ios_base::ate);

        if(!file_stream.is_open())
        {
            model_vertices.emplace_back(Vertex{});
            model_indicies.emplace_back(0);
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

        std::vector<Vertex>& vertex = model_vertices;
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

        for (size_t i = index; i < file_size; i++){
            if (file[i] == '\n') {
                for (size_t t = i; t < file_size; t++)
                {
                    char prev = file[t-1];
                    char current = file[t];
                    char next = file[t+1];
                    char next2 = file[t+2];
                    if(file[t] == 'f'){
                        i = t-1;
                        break;
                    }
                }
            }
            if(file[i] == ' '){
                Indices triangle_indexes {};
                triangle_indexes.vertex_index = static_cast<uint32_t>(atoi(&file[i+1]));
                size_t y = i;
                for(size_t x = y; x < file_size; x++){
                    if(file[x] == '\n') break;
                    if(file[x] == '/'){
                        triangle_indexes.texture_index = static_cast<uint32_t>(atoi(&file[x+1]));
                        y = x+1;
                        break;
                    }
                }
                for(size_t x = y; x < file_size; x++){
                    if(file[x] == '\n') break;
                    if(file[x] == '/'){
                        triangle_indexes.normal_index = static_cast<uint32_t>(atoi(&file[x+1]));
                        break;
                    }
                }
                indicies.emplace_back(triangle_indexes);
            }
        }

        model_indicies.reserve(indicies.size());
        for (size_t i = 0; i < indicies.size(); i++)
        {
            uint32_t val = indicies[i].vertex_index;
            uint32_t bal = indicies[i].texture_index;
            if(val > 0 && bal > 0){
                vertex[indicies[i].vertex_index-1].texture_cord = texture_cords[indicies[i].texture_index-1];
                model_indicies.push_back(indicies[i].vertex_index-1);
            }
        }

        //model_vertices.resize(vertex.size());
        //memcpy(model_vertices.data(), vertex.data(), vertex.size() * sizeof(Vertex));
        // vertices = vertex;
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

    static void de_serialize(const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
    {
        std::ifstream file(filename, std::ios::binary);

        if(!file.is_open()){
            Debug::log((char*)"There was a issue parsing this model");
            return;
        }

        //Find out where the file ends
        file.seekg (0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0);

        //Figures out where the vertexes starts/stops
        size_t index_start = 0;
        file.read(reinterpret_cast<char*>(&index_start), sizeof(uint32_t));

        //Prepare and then load all vertexes into the vertex buffer
        vertices.resize(index_start / sizeof(Vertex));
        file.read(reinterpret_cast<char*>(vertices.data()), index_start);

        //Prepare and then load all indices into the index buffer
        indices.resize(file_size - index_start);
        file.read(reinterpret_cast<char*>(indices.data()), file_size - index_start);
        file.close();
    }

    static Model create_model(Device& device, VkCommandPool command_pool, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
    {
        Model result {};
        result.index_amount = indices.size();
        CommandBuffer::create_vertex_buffer(device, vertices, result.vertex_buffer, result.vertex_buffer_memory, command_pool);
        CommandBuffer::create_index_buffer(device, indices, result.index_buffer, result.index_buffer_memory, command_pool);
        return result;
    }

    Model load_model(Device& device, VkCommandPool command_pool, std::string filename)
    {
        Model model;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        char extention[3];
        extention[0] = filename[filename.length() -3];
        extention[1] = filename[filename.length() -2];
        extention[2] = filename[filename.length() -1];

        if(extention[0] == 'o' || extention[0] == 'O'){
            parse_obj(filename.c_str(), vertices, indices);
        }else if(extention[0] == 'b' || extention[0] == 'B'){
            de_serialize(filename.c_str(), vertices, indices);
        }

        model = create_model(device, command_pool, vertices, indices);

        return model;
    }
}/**/
