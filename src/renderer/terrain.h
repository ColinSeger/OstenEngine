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

static inline void create_terrain(int width, int depth, Terrain* terrain){
    terrain->height = depth;
    terrain->width = width;

    terrain->vertexes.amount = width * depth;
    terrain->indexes.amount  = (width - 1) * (depth - 1) * 6;

    terrain->height_map = (float*)malloc(sizeof(float) * terrain->vertexes.amount);//Fix later
    terrain->vertexes.values = (Vertex*)malloc(sizeof(Vertex) * terrain->vertexes.amount);
    terrain->indexes.values = (uint32_t*)malloc(sizeof(uint32_t) * terrain->indexes.amount);

    // Generate heights sine wave terrain
    for (int z = 0; z < depth; z++){
        for (int x = 0; x < width; x++){
            int i = z * width + x;

            float height = sinf(x * 0.2f) * cosf(z * 0.2f) * 2.0f;

            terrain->height_map[i] = height;

            terrain->vertexes.values[i].position.x = (float)x;
            terrain->vertexes.values[i].position.y = height;
            terrain->vertexes.values[i].position.z = (float)z;
        }
    }

    // Generate triangle indices
    int index = 0;
    for (int z = 0; z < depth - 1; z++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            int topLeft     =  z      * width + x;
            int topRight    =  topLeft + 1;
            int bottomLeft  = (z + 1) * width + x;
            int bottomRight =  bottomLeft + 1;

            // Triangle 1
            terrain->indexes.values[index++] = topLeft;
            terrain->indexes.values[index++] = bottomLeft;
            terrain->indexes.values[index++] = topRight;

            // Triangle 2
            terrain->indexes.values[index++] = topRight;
            terrain->indexes.values[index++] = bottomLeft;
            terrain->indexes.values[index++] = bottomRight;
        }
    }

}

static inline void create_terrain_mesh(Terrain terrain, RenderPipeline* render_pipe){
    Model model = {};
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
    if(x * y > terrain.width * terrain.height) return terrain.height_map[terrain.width * terrain.height -1];
    if(x < 0 || y < 0) return terrain.height_map[0];
    return terrain.height_map[y * terrain.width + x];
}
