#pragma once
#include <vector>
#include <stdint.h>
#include "../../renderer/render_pipeline.cpp"
#include "../../renderer/model_loader/model_loader.cpp"
#include "../entity_manager/entity_manager.cpp"

enum class MessageType : uint8_t
{
    None,
    LoadModel,
    CreateEntity,
    LoadTexture,
    SerializeOBJ,
    CreateRenderable,
    TestEntity
};

enum class SupportedFiles : uint8_t
{
    bin,
    obj,
    png,
    jpg
};

struct InstanceData{
    int32_t capacity;
    uint16_t model_index;
    uint16_t texture_index;
};

static void create_entity(const char* name){
    Entity entity{};
    EntityManager::add_entity(entity, name);
}

static void create_renderable(struct RenderPipeline* render_pipeline, InstanceData* asset_index, HeapStack* heap_stack){
    //uint16_t transform_id = add_transform();
    for(uint16_t transform = 0; transform < asset_index->capacity; transform++){
        add_transform();
    }
    uint32_t index = 0;
    RenderAble* renderable = get_free_renderable(render_pipeline->model_render_data, heap_stack, &index);
    renderable->model_index = asset_index->model_index;
    renderable->texture_index = asset_index->texture_index;
    renderable->capacity = asset_index->capacity;
    renderable->instance_amount = 0;
    renderable->transform_index = arena_alloc_memory(heap_stack, sizeof(uint16_t) * renderable->capacity);

    create_model_set(render_pipeline->device.virtual_device, render_pipeline->descriptor_pool, render_pipeline->model_set_layout, render_pipeline->model_render_data, index, heap_stack);

    render_pipeline->model_render_data.renderable_amount++;
}

static void load_asset(const char* file_name, struct RenderPipeline& render_pipeline, HeapStack* memory_arena)
{
    std::string filename = file_name;
    char extention[3];
    extention[0] = filename[filename.length() -3];
    extention[1] = filename[filename.length() -2];
    extention[2] = filename[filename.length() -1];

    if(extention[0] == 'o' || extention[0] == 'O'){
        ModelLoader::load_model(render_pipeline.device, render_pipeline.command_pool, file_name, LoadMode::OBJ, memory_arena);
    }else if(extention[0] == 'b' || extention[0] == 'B'){
        ModelLoader::load_model(render_pipeline.device, render_pipeline.command_pool, file_name, LoadMode::BIN, memory_arena);
    }else if(extention[0] == 'p' || extention[0] == 'P'){
        uint32_t texture_index = Texture::load_texture(render_pipeline.device, file_name, render_pipeline.command_pool);
        create_fragment_set2(render_pipeline.device.virtual_device, render_pipeline.descriptor_pool, render_pipeline.fragment_layout, render_pipeline.texture_descriptor, render_pipeline.shadow_pass.image_view, render_pipeline.shadow_pass.sampler, render_pipeline.light_position, texture_index);
    }
}

/// Size, Type, Value
struct Message
{
    uint32_t size;
    MessageType type = MessageType::None;
    void* value;
};

struct MessageSystem
{
    std::vector<Message> messages;
};

struct TempComp{
    uint16_t entity;
    uint16_t component;
};

static std::vector<Message> messages;
static std::vector<Entity> entities_to_create;
/*
static uint16_t entity_queue = 0;

static std::vector<TempComp> component_stack;
 *
static uint16_t queue_enity_creation(){//Actually Stack
    entities_to_create.emplace_back(Entity{});
    entity_queue++;
    return entity_queue-1;
}

static void queue_component_add(uint16_t entity_id, uint16_t component_to_add){//Actually Stack
    component_stack.push_back({entity_id, component_to_add});
}


static void handle_entity(){
    for(int i = 0; i < entity_queue; i++){
        Entity entity{};
        for(int x = 0; x < component_stack.size(); x++){
            if(i != component_stack[x].entity) continue;

            switch (component_stack[x].component){
            case CAMERA:
                //add_camera(uint16_t transform_index);
            break;
            case TRANSFORM:
                add_transform();
            break;
            case RENDER:

            break;
            case COLLIDER:

            break;
            default:
            break;
            }
            add_component(entity, {component_stack[x].component});
        }
        EntityManager::add_entity(entity, "G");
    }
    Entity& entity = entities_to_create[0];

    entities_to_create.erase(entities_to_create.begin());
}
*/

static void add_message(Message message){
    messages.emplace_back(message);
}
static void handle_message(struct RenderPipeline* render_pipeline, HeapStack* heap_stack){
    if(messages.size() <= 0) return;
    Message message = messages.front();
    char* action = reinterpret_cast<char*>(message.value);
    //Entity ent = *(Entity*)message.value;

    switch (message.type)
    {
    case MessageType::LoadModel :
        load_asset(action, *render_pipeline, heap_stack);
    break;

    case MessageType::CreateEntity :
        create_entity(action);
    break;
    case MessageType::LoadTexture :
        load_asset(action, *render_pipeline, heap_stack);
    break;
    case MessageType::SerializeOBJ :
        ModelLoader::serialize(action, heap_stack);
    break;
    case MessageType::CreateRenderable:
        create_renderable(render_pipeline, (InstanceData*)message.value, heap_stack);
    break;
    case MessageType::TestEntity:

    break;
    default:
        break;
    }
    messages.erase(messages.begin());
}

static void procces_all_commands(struct RenderPipeline* render_pipeline, HeapStack* heap_stack){
    while (!messages.empty()) {
        handle_message(render_pipeline, heap_stack);
    }
    for(Entity e : entities_to_create){
        EntityManager::add_entity(e, "Game Object");
    }
    entities_to_create.clear();
}
