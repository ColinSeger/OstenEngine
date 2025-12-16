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

    static inline ObjMode select_mode(char* char_to_check)//This is ass
    {
        if(*char_to_check == '#'){
            return ObjMode::Comment;
        }
        if (*char_to_check == 'v') {
            char_to_check++;
            if(*char_to_check == ' '){
                return ObjMode::Vertex;
            }
            if(*char_to_check == 't'){
                return ObjMode::TextureCord;
            }
        }
        if(*char_to_check == 'f'){
            return ObjMode::Face;
        }
        return ObjMode::None;
    }

    static inline void next_valid(char* file, size_t* current_value, size_t max_value){
        for (size_t i = *current_value; i < max_value; i++){
            if (file[i] == '\n' && file[i+1] == 'v'){
                *current_value = i+1;
                break;
            }
        }
    }

    static void parse_obj(const char* path_of_obj, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
    {
        std::ifstream file_stream(path_of_obj, std::ios_base::in | std::ios_base::ate);

        if(!file_stream.is_open())
        {
            vertices.emplace_back(Vertex{});
            indices.emplace_back(0);
            Debug::log((char*)"Failed to load model");
            return;
        }

        size_t file_size = file_stream.tellg();
        file_stream.seekg(0);
        char* file = (char*)malloc(sizeof(char) * file_size);

        file_stream.read(file, file_size);
        file_stream.close();

        std::vector<Vertex> vertex;
        std::vector<Indices> indicies;
        std::vector<TextureCord> texture_cords;

        ObjMode current_mode = ObjMode::None;
        std::string values[3];
        union {
            float vertex_to_add[3];
            float indicies_to_add[3];
            float texture_cord[2];
        };
        // float vertex_to_add[3];

        uint8_t char_index = 0;

        vertex.reserve(file_size/40);
        texture_cords.reserve(file_size/60);
        indicies.reserve(file_size/30);

        size_t index = 0;
/*

        next_valid(file, &index, file_size);

        for (size_t i = index; i < file_size; i++){
            if (file[i] == '\n') {
                vertex.emplace_back(Vertex{{vertex_to_add[0], vertex_to_add[1], vertex_to_add[2]}, {0, 0, 0}, {0, 0}});
                char_index = 0;
                if(file[i+1] != 'v' || file[i+2] != ' '){
                    index = i;
                    break;
                }
            }
            if(file[i] == ' '){
                vertex_to_add[char_index] = atof(&file[i+1]);
                char_index ++;
            }
        }

        next_valid(file, &index, file_size);

        for (size_t i = index; i < file_size; i++){
            if (file[i] == '\n') {
                texture_cords.emplace_back(texture_cord[0], texture_cord[1]);
                if(file[i+1] != 'v' || file[i+2] != 't'){
                    index = i;
                    break;
                }
            }
            if(file[i] == ' '){
                texture_cord[char_index] = atof(&file[i+1]);
                char_index ++;
            }
        }

        next_valid(file, &index, file_size);

        for (size_t i = index; i < file_size; i++){
            if (file[i] == '\n') {
                // vertex.emplace_back(Vertex{{vertex_to_add[0], vertex_to_add[1], vertex_to_add[2]}, {0, 0, 0}, {0, 0}});
                if(file[i+1] != 'v' || file[i+2] != 'n'){
                    index = i;
                    break;
                }
            }
            if(file[i] == ' '){
                // vertex_to_add[char_index] = atof(&file[i+1]);
                char_index ++;
            }
        }

        next_valid(file, &index, file_size);

        for (size_t i = index; i < file_size; i++){
            if (file[i] == '\n') {
                // vertex.emplace_back(Vertex{{vertex_to_add[0], vertex_to_add[1], vertex_to_add[2]}, {0, 0, 0}, {0, 0}});
                if(file[i+1] != 'f' || file[i+2] != ' '){
                    index = i;
                    break;
                }
            }
            if(file[i] == '/'){
                // vertex_to_add[char_index] = atof(&file[i+1]);
                char_index ++;
            }
        }

        for (size_t i = 0; i < file_size; i++)
        {
            char value = file[i];

            if(value == '\n' || current_mode == ObjMode::None){

                switch (current_mode)
                {
                case ObjMode::None:
                    current_mode = select_mode(&file[i]);
                    if(current_mode == ObjMode::TextureCord) i++;//do i even need to?
                    char_index = 0;
                    continue;
                break;
                case ObjMode::Comment:

                break;
                case ObjMode::Vertex:
                    //vertex.emplace_back(Vertex{{vertex_to_add[0], vertex_to_add[1], vertex_to_add[2]}, {0, 0, 0}, {0, 0}});
                break;
                case ObjMode::Face:
                    for (std::string& index : values)
                    {
                        if(index.length() <= 0) continue;
                        uint32_t vertex_index = std::stoi(index)-1;
                        for (size_t i = 0; i < index.size(); i++)
                        {
                            if(index[i] == '/')
                            {
                                Indices indecies;
                                indecies.vertex_index = vertex_index;
                                indecies.texture_index = std::stoi(&index[i+1])-1;
                                indicies.emplace_back(indecies);
                                break;
                            }
                        }
                    }
                break;
                case ObjMode::TextureCord:
                    //texture_cord.emplace_back(TextureCord{std::stof(values[0]), 1.f - std::stof(values[1])});
                break;
                case ObjMode::Normal:
                    //Todo
                break;
                default:
                    break;
                }
                for (std::string& index : values)
                {
                    index.clear();
                }
                current_mode = ObjMode::None;
                continue;
            }

            if(current_mode == ObjMode::None || current_mode == ObjMode::Comment) continue; // do I even need this?

            if(value == ' '){
                if(current_mode == ObjMode::Vertex){
                    vertex_to_add[char_index] = atof(&file[i+1]);
                }
                char_index++;
                continue;
            }
            else if (is_valid_char(value)){

                //values[char_index].push_back(value);
            }
        }
 */
        indices.reserve(indicies.size());
        for (size_t i = 0; i < indicies.size(); i++)
        {
            vertex[indicies[i].vertex_index].texture_cord = texture_cords[indicies[i].texture_index];
            indices.emplace_back(indicies[i].vertex_index);
        }
        vertices = vertex;

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
