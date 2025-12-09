#pragma once
#include <vector>
#include <cstdint>
#include "../entity_manager/entity_manager.h"
#include "../../renderer/model_loader/model_loader.h"

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
void add_message(MessageSystem& system, Message message);

void handle_message(MessageSystem& system);
