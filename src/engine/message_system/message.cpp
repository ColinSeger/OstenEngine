#pragma once
#include <vector>
#include <cstdint>
// #include "../entity_manager/entity_manager.cpp"
// #include "../../renderer/model_loader/model_loader.cpp"

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

void add_message(MessageSystem& system, Message message){

}

void handle_message(MessageSystem& system){
    if(system.messages.size() > 0) return;
    Message message = system.messages.front();

    switch (message.type)
    {
    case MessageType::Load :
        // ModelLoader::load_model()
        break;

    default:
        break;
    }
}
