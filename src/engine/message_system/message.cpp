#pragma once
#include <vector>
#include <cstdint>
#include "../../renderer/render_pipeline.cpp"
#include "../../renderer/model_loader/model_loader.cpp"

enum class MessageType : uint8_t
{
    None,
    Load
};

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

void handle_message(RenderPipeline& render_pipeline){
    if(messages.size() <= 0) return;
    Message message = messages.front();
    char* action = reinterpret_cast<char*>(message.value);

    switch (message.type)
    {
    case MessageType::Load :
        ModelLoader::load_model(render_pipeline.device, render_pipeline.command_pool, action);
        break;

    default:
        break;
    }
}
