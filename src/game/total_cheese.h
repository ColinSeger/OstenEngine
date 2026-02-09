#pragma once
#include "../engine/message_system/message.h"
#include "../../external/math_3d.h"

static InstanceData render_ids {};
static std::string name = std::string("Game Object ");

static void init_game(){
    Message message{};
    message.type = MessageType::LoadModel;
    message.value = (void*)"assets/debug_assets/viking.bin";

    add_message(message);

    message.type = MessageType::LoadTexture;
    message.value = (void*)"assets/debug_assets/viking_room.png";

    add_message(message);
    render_ids.model_index = 0;
    render_ids.texture_index = 1;
    render_ids.capacity = 1000;

    message.type = MessageType::CreateRenderable;
    message.value = (void*)&render_ids;

    add_message(message);

    char buf[11];

    for(int i = 0; i < 999; i++){
        snprintf(buf, sizeof(buf), "%u", i);
        for(char c : buf){
            if(c == 0) break;
            name.push_back(c);
        }

        message.type = MessageType::CreateEntity;
        message.value = (void*)&"";

        add_message(message);

    }
}

static void run_game(){

}
