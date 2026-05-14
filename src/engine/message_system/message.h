#pragma once
#include <string.h>
//#include <vector>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include "../../renderer/render_pipeline.h"
//#include "../../renderer/model_loader/model_loader.cpp"
//#include "../entity_manager/entity_manager.cpp"

typedef enum MessageType
{
    None,
    LoadModel,
    CreateEntity,
    LoadTexture,
    SerializeOBJ,
    CreateRenderable,
    TestEntity,
    MessageTypeSize
} MessageType;

typedef enum SupportedFiles
{
    bin,
    obj,
    png,
    jpg
} SupportedFiles;

typedef struct InstanceData{
    int32_t capacity;
    uint16_t model_index;
    uint16_t texture_index;
} InstanceData;

static void create_entity(const char* name){
    //Entity entity{};
    //add_entity(entity, name);
}

static inline void create_renderable(struct RenderPipeline* render_pipeline, InstanceData* asset_index, HeapStack* heap_stack){

    vkDeviceWaitIdle(render_pipeline->device.virtual_device);
    //uint16_t transform_id = add_transform();
    // for(uint16_t transform = 0; transform < asset_index->capacity; transform++){
    //     //add_transform();
    // }
    uint32_t index = 0;
    RenderAble* renderable = get_free_renderable(&render_pipeline->model_render_data, heap_stack, &index);
    renderable->model_index = asset_index->model_index;
    renderable->texture_index = asset_index->texture_index;
    renderable->capacity = asset_index->capacity;
    renderable->instance_amount = 0;
    renderable->transform_index = arena_alloc_memory(heap_stack, sizeof(uint16_t) * renderable->capacity);

    create_model_set(render_pipeline->device.virtual_device, render_pipeline->descriptor_pool, render_pipeline->model_set_layout, &render_pipeline->model_render_data, index, heap_stack);

    render_pipeline->model_render_data.renderable_amount++;
}

static void load_asset(const char* file_name, struct RenderPipeline* render_pipeline, HeapStack* memory_arena){
    if(file_name[0] == '\0') return;
    // std::string filename = file_name;
    // char extention[3];
    // extention[0] = filename[filename.length() -3];
    // extention[1] = filename[filename.length() -2];
    // extention[2] = filename[filename.length() -1];

    // if(extention[0] == 'o' || extention[0] == 'O'){
    //     //ModelLoader::load_model(render_pipeline.device, render_pipeline.command_pool, file_name, LoadMode::OBJ, memory_arena);
    // }else if(extention[0] == 'b' || extention[0] == 'B'){
    //     //ModelLoader::load_model(render_pipeline.device, render_pipeline.command_pool, file_name, LoadMode::BIN, memory_arena);
    // }else if(extention[0] == 'p' || extention[0] == 'P' || extention[0] == 'j'){
    //     uint32_t texture_index = load_texture(&render_pipeline.device, (char*)file_name, render_pipeline.command_pool).index;
    //     create_fragment_set2(render_pipeline.device.virtual_device, render_pipeline.descriptor_pool, render_pipeline.fragment_layout, &render_pipeline.texture_descriptor, &render_pipeline.lights, texture_index);
    // }
}

/// Size, Type, Value
typedef struct Message
{
    MessageType type;
    uint32_t size;
    void* value;
} Message;

typedef struct MessageSystem{
    HeapStack memory_arena;
    //std::vector<Message> messages;
} MessageSystem;

typedef struct TempComp{
    uint16_t entity;
    uint16_t component;
} TempComp;

static Message messages[255] = {};
static uint32_t message_amount = 0;


static inline void add_message(Message message){
    //void* test = malloc(message.size);
    //memcpy(message.value, message.value, message.size);

    messages[message_amount] = message;
}

static inline Message create_message(MessageType type, uint8_t size, char value[255]){
    Message message = {};
    memcpy(message.value, value, message.size);
    return message;
}

static inline void add_message_f(MessageType type, uint8_t size, char value[255]){
    return;
    Message message = {};
    message.type = type;
    message.size = size;
    memcpy(message.value, value, message.size);
    messages[message_amount] = message;
    message_amount++;
}

static inline void procces_all_commands(struct RenderPipeline* render_pipeline, HeapStack* heap_stack){
    for(uint32_t i = 0; i < message_amount; i++){
        Message message = messages[i];
        char* action = (char*)message.value;
        //Entity ent = *(Entity*)message.value;

        switch (message.type)
        {
        case LoadModel :
            load_asset(action, render_pipeline, heap_stack);
        break;

        case CreateEntity :
            create_entity(action);
        break;
        case LoadTexture :
            load_asset(action, render_pipeline, heap_stack);
        break;
        case SerializeOBJ :
            //ModelLoader::serialize(action, heap_stack);
        break;
        case CreateRenderable:
            create_renderable(render_pipeline, (InstanceData*)message.value, heap_stack);
        break;
        case TestEntity:

        break;
        default:
            break;
        }

    }
    memset(messages, 0, sizeof(messages));
    message_amount = 0;
}
