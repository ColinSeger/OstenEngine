#pragma once
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include "device/vulkan/device.h"
#include "render_pipeline.cpp"
#include "model_loader/model_loader.cpp"

struct Terrain{//Bad
    VertexArray vertexes;
    Uint32Array indexes;
    float* height_map;
    uint16_t height;
    uint16_t width;
};

static inline unsigned long long create_terrain(int width, int depth, Terrain* terrain, HeapStack* heap_stack){
    unsigned long long mem_index = heap_stack->index;
    terrain->height = depth;
    terrain->width = width;

    terrain->vertexes.amount = width * depth;
    terrain->indexes.amount  = (width - 1) * (depth - 1) * 6;

    unsigned long long height_map = arena_alloc_memory(heap_stack, sizeof(float) * terrain->vertexes.amount);
    unsigned long long vertex_values = arena_alloc_memory(heap_stack, sizeof(Vertex) * terrain->vertexes.amount);
    unsigned long long index_values = arena_alloc_memory(heap_stack, sizeof(uint32_t) * terrain->indexes.amount);
    mem_index = vertex_values;

    terrain->height_map = (float*)get_at_index(heap_stack, height_map); //(float*)malloc(sizeof(float) * terrain->vertexes.amount);//Fix later
    terrain->vertexes.values = (Vertex*)get_at_index(heap_stack, vertex_values);
    terrain->indexes.values = (uint32_t*)get_at_index(heap_stack, index_values);

    // Generate heights sine wave terrain
    for (int x = 0; x < depth; x++){
        for (int y = 0; y < width; y++){
            int i = x * width + y;

            float height = sinf(y * 0.2f) * cosf(x * 0.2f) * 2.0f;

            terrain->height_map[i] = height;

            terrain->vertexes.values[i].position.x = (float)x;
            terrain->vertexes.values[i].position.y = (float)y;
            terrain->vertexes.values[i].position.z = height;

            terrain->vertexes.values[i].texture_cord.x = (float)x / (float)depth;
            terrain->vertexes.values[i].texture_cord.y = (float)y / (float)width;
        }
    }

    for (int x = 0; x < depth; x++){
        for (int y = 0; y < width; y++){
            int triangle = x * width + y;
            int triangle1 = (1+x) * width + y;
            int triangle2 = x * width + (y+1);
            if(triangle1 > depth * width) triangle1 = depth * width;
            if(triangle2 > depth * width) triangle2 = depth * width;

            terrain->vertexes.values[triangle].normals = v3_cross(terrain->vertexes.values[triangle1].position, terrain->vertexes.values[triangle2].position);

        }
    }

    // Generate triangle indices
    int index = 0;
    for (int x = 0; x < depth - 1; x++) {
        for (int y = 0; y < width - 1; y++){
            int top_left =  x * width + y;
            int top_right =  top_left + 1;
            int bottom_left = (x + 1) * width + y;
            int bottom_right = bottom_left + 1;

            // Triangle 1
            terrain->indexes.values[index++] = top_left;
            terrain->indexes.values[index++] = bottom_left;
            terrain->indexes.values[index++] = top_right;

            // Triangle 2
            terrain->indexes.values[index++] = top_right;
            terrain->indexes.values[index++] = bottom_left;
            terrain->indexes.values[index++] = bottom_right;
        }
    }
    return mem_index;
}

static inline void create_terrain_mesh(Terrain terrain, RenderPipeline* render_pipe){
    Model model = {};
    model.index_amount = terrain.indexes.amount;
    VkResult vertex = CommandBuffer::create_vertex_buffer(&render_pipe->device, &terrain.vertexes, &model.vertex_buffer, &model.vertex_buffer_memory, render_pipe->command_pool);
    VkResult index = CommandBuffer::create_index_buffer(&render_pipe->device, &terrain.indexes, &model.index_buffer, &model.index_buffer_memory, render_pipe->command_pool);

    if(vertex != VK_SUCCESS || index != VK_SUCCESS) {
        throw "Failed to create";
    }

    loaded_models.emplace_back(model);
    loaded_model_index["Terrain"] = loaded_models.size() -1;

 //   ModelLoader::serialize2(terrain.vertexes, terrain.indexes);
}

static inline float sample_terrain_height(Terrain terrain, int x, int y){
    if(x * y > terrain.width * terrain.height) return terrain.height_map[terrain.width * terrain.height -2];
    if(x < 0 || y < 0) return terrain.height_map[0];
    return terrain.height_map[y * terrain.width + x] +1;
}

static inline float sample_terrain_height_interpolated(Terrain terrain, float x, float y){
    if(x * y > terrain.width * terrain.height) return terrain.height_map[(terrain.width - 1) * (terrain.height - 1)];
    if(x < 0 || y < 0) return terrain.height_map[0];

    int x_int = (int)x;
    int y_int = (int)y;

    int x_int_corner = x_int + 1;
    int y_int_corner = y_int + 1;

    // Fractional part
    float time_x_axis = x - x_int;
    float time_y_axis = y - y_int;

    // Sample four corners
    float corner_1 = terrain.height_map[y_int * terrain.width + x_int];
    float corner_2 = terrain.height_map[y_int * terrain.width + x_int_corner];
    float corner_3 = terrain.height_map[y_int_corner * terrain.width + x_int];
    float corner_4 = terrain.height_map[y_int_corner * terrain.width + x_int_corner];

    float hx0 = interpolate_f(corner_1, corner_2, time_x_axis);
    float hx1 = interpolate_f(corner_3, corner_4, time_x_axis);

    return interpolate_f(hx0, hx1, time_y_axis);
}
