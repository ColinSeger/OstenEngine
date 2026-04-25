#pragma once
#include <stddef.h>
#include <stdint.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include <vector>
#include "../device/vulkan/device.h"
//#include "../../debugger/debugger.h"
#include "obj_parser.h"

/**
    The representation models used by this engine.
*/
struct Model{
    uint32_t index_amount;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;//TODO Look into how to merge into vertex buffer
    VkDeviceMemory index_buffer_memory;
};

enum class LoadMode{
    OBJ,
    BIN
};

// std::unordered_map<std::string, uint32_t> loaded_model_index;
// std::vector<Model> loaded_models;

namespace ModelLoader
{
    static void serialize(const char* filename, HeapStack* memory_arena){
        VertexArray vertices;
        Uint32Array indices;

        std::string new_name = filename;

        new_name[new_name.size()-1] = 'n';
        new_name[new_name.size()-2] = 'i';
        new_name[new_name.size()-3] = 'b';

        size_t to_free = load_obj_v2(filename, vertices, indices, memory_arena);

        std::ofstream file(new_name, std::ios::binary);

        if(!file.is_open()){
            abort();
        }
        uint32_t index_start = vertices.amount * sizeof(Vertex);

        file.write(reinterpret_cast<const char*>(&index_start), sizeof(uint32_t));

        file.write(reinterpret_cast<char*>(vertices.values), index_start);

        file.write(reinterpret_cast<char*>(indices.values),  indices.amount * sizeof(uint32_t));

        file.close();
        free_arena(memory_arena, to_free);
    }

    static inline void serialize2(VertexArray vertices, Uint32Array indices){

        std::ofstream file("new_name.bin", std::ios::binary);

        if(!file.is_open()){
            abort();
        }
        uint32_t index_start = vertices.amount * sizeof(Vertex);

        file.write(reinterpret_cast<const char*>(&index_start), sizeof(uint32_t));

        file.write(reinterpret_cast<char*>(vertices.values), index_start);

        file.write(reinterpret_cast<char*>(indices.values),  indices.amount * sizeof(uint32_t));

        file.close();
    }

    //This Returns a size_t from the memory arena index
    static size_t de_serialize(const char* filename, VertexArray& vertices, Uint32Array& indices, HeapStack* heap_stack){
        //Debug::profile_time_start();
        std::ifstream file(filename, std::ios::binary | std::ios::ate);

        if(!file.is_open()){
            //Debug::log((char*)"There was a issue parsing this model");
            return heap_stack->index;
        }

        //Find out where the file ends
        size_t file_size = file.tellg();
        file.seekg(0);
        size_t mem_index = arena_alloc_memory(heap_stack, file_size);
        char* buffer = (char*)get_at_index(heap_stack, mem_index);

        file.read(buffer, file_size);
        file.close();
        //195528 debug asset

        //Figures out where the vertexes starts/stops
        uint32_t index_start = *((uint32_t*)buffer);

        //Sets where to start values of the model
        vertices.values = (Vertex*)(buffer + sizeof(uint32_t));
        indices.values = (uint32_t*)(buffer+ sizeof(uint32_t) + index_start);

        //Sets amount the array contains
        vertices.amount = index_start / sizeof(Vertex);
        indices.amount = (file_size - index_start) / sizeof(uint32_t);

        //Debug::profile_time_end();
        return mem_index;
    }

    uint32_t load_model(Device& device, VkCommandPool command_pool, const char* file_name, LoadMode load_mode, HeapStack* memory_arena){
        auto contains = loaded_model_index.find(file_name);

        if(contains != loaded_model_index.end()){
            return loaded_model_index[file_name];
        }
        size_t file_to_free = memory_arena->index;//Temp solution for speed test
        VertexArray vertices;
        Uint32Array indices;

        if(load_mode == LoadMode::OBJ){
            //parse_obj(file_name, vertices, indices, memory_arena);
            file_to_free = load_obj_v2(file_name, vertices, indices, memory_arena);
        }else if(load_mode == LoadMode::BIN){
            file_to_free = de_serialize(file_name, vertices, indices, memory_arena);
        }

        if(file_to_free == memory_arena->index) return 0;
        Model model{};

        model.index_amount = indices.amount;
        CommandBuffer::create_vertex_buffer(&device, &vertices, &model.vertex_buffer, &model.vertex_buffer_memory, command_pool);
        CommandBuffer::create_index_buffer(&device, &indices, &model.index_buffer, &model.index_buffer_memory, command_pool);
        loaded_models.emplace_back(model);
        loaded_model_index[file_name] = loaded_models.size() -1;

        free_arena(memory_arena, file_to_free);

        return loaded_model_index[file_name];
    }
}/**/
