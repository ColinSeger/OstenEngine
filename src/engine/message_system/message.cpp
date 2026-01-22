#pragma once
#include <vector>
#include <cstdint>
#include "../../renderer/render_pipeline.cpp"
#include "../../renderer/model_loader/model_loader.cpp"
#include "../entity_manager/entity_manager.cpp"

enum class MessageType : uint8_t
{
    None,
    LoadModel,
    CreateEntity,
    LoadTexture,
    SerializeOBJ
};

static void create_entity(struct RenderPipeline* render_pipeline, const char* name){
     vkDeviceWaitIdle(render_pipeline->device.virtual_device);//TODO have actual solution for this instead of waiting for device idle
    render_pipeline->render_data.render_descriptors.emplace_back();

    create_uniform_buffer(render_pipeline->render_data.render_descriptors.back(), render_pipeline->device);

    uint32_t index = Texture::load_texture(render_pipeline->device, ".png", render_pipeline->command_pool);
    TextureImage texture = loaded_textures[index];

    create_descriptor_set(render_pipeline->device.virtual_device, render_pipeline->render_data.render_descriptors.back(), render_pipeline->camera_descript, render_pipeline->descriptor_pool, render_pipeline->descriptor_set_layout, texture.image_view, texture.texture_sampler, render_pipeline->shadow_pass.image_view, render_pipeline->shadow_pass.sampler);
    create_shadow_sets(render_pipeline->device.virtual_device, render_pipeline->light, render_pipeline->descriptor_pool, render_pipeline->shadow_layout);

    Entity entity{};
    EntityManager::add_entity(entity, name);
}

void load_asset(const char* file_name, struct RenderPipeline& render_pipeline, MemArena& memory_arena)
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
        Texture::load_texture(render_pipeline.device, file_name, render_pipeline.command_pool);
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

std::vector<Message> messages;

void add_message(Message message){
    messages.emplace_back(message);
}

void handle_message(struct RenderPipeline* render_pipeline, MemArena& memory_arena){
    if(messages.size() <= 0) return;
    Message message = messages.front();
    char* action = reinterpret_cast<char*>(message.value);

    switch (message.type)
    {
    case MessageType::LoadModel :
        load_asset(action, *render_pipeline, memory_arena);
    break;

    case MessageType::CreateEntity :
        create_entity(render_pipeline, action);
    break;
    case MessageType::LoadTexture :
        load_asset(action, *render_pipeline, memory_arena);
    break;
    case MessageType::SerializeOBJ :
        ModelLoader::serialize(action, memory_arena);
    break;
    default:
        break;
    }
    messages.erase(messages.begin());
}
