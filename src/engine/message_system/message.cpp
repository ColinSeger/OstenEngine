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
    LoadTexture
};

static void create_entity(RenderPipeline* render_pipeline){
    //uint16_t id = add_transform();
    RenderDescriptors first_obj;
    //first_obj.transform_index = id;
    //first_obj.model_index = render_pipeline->models.size() -1;

    render_pipeline->create_uniform_buffer(first_obj);
    TextureImage texture;
    if(render_pipeline->to_render.size() < 2){
        uint32_t test = Texture::load_texture(render_pipeline->device, "assets/debug_assets/viking_room.png", render_pipeline->command_pool);
        texture = loaded_textures[test];
    }else{
        uint32_t test = Texture::load_texture(render_pipeline->device, "assets/debug_assets/viking_room.png", render_pipeline->command_pool);
        texture = loaded_textures[test];
    }

    create_descriptor_set(render_pipeline->device.virtual_device, first_obj, render_pipeline->descriptor_pool, render_pipeline->descriptor_set_layout, texture.image_view, texture.texture_sampler);
    render_pipeline->to_render.push_back(first_obj);

    Entity entity{};
    // entity.components.push_back({id, TRANSFORM});
    EntityManager::add_entity(entity, "Test");
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

void handle_message(RenderPipeline* render_pipeline){
    if(messages.size() <= 0) return;
    Message message = messages.front();
    char* action = reinterpret_cast<char*>(message.value);

    switch (message.type)
    {
    case MessageType::LoadModel :
        render_pipeline->models.emplace_back(ModelLoader::load_model(render_pipeline->device, render_pipeline->command_pool, action));
        break;

    case MessageType::CreateEntity :
        create_entity(render_pipeline);
    break;
    case MessageType::LoadTexture:
        Texture::load_texture(render_pipeline->device, action, render_pipeline->command_pool);
    break;
    default:
        break;
    }
    messages.erase(messages.begin());
}
