#pragma once
#include "../engine/message_system/message.h"
#include "../../external/math_3d.h"
#include "../osten_engine.cpp"
#include <cmath>
#include <cstdint>

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
    constexpr uint32_t capacity = 2000;
    render_ids.model_index = 0;
    render_ids.texture_index = 1;
    render_ids.capacity = capacity +2;

    message.type = MessageType::CreateRenderable;
    message.value = (void*)&render_ids;

    add_message(message);

    char buf[11];

    for(int i = 0; i < capacity; i++){
        snprintf(buf, sizeof(buf), "%u", i);
        for(char c : buf){
            if(c == 0) break;
            name.push_back(c);
        }

        message.type = MessageType::CreateEntity;
        message.value = (void*)&"Game Object";

        add_message(message);

    }
}
static double test = 0;
static void update_game(double delta_time, OstenEngine& engine){
    test+= delta_time;
    RenderAble* rendera = get_renderable(engine.render_pipeline.model_render_data, 0, engine.heap_stack);
    for (uint16_t i = 0; i < rendera->instance_amount-1; i++) {
        TransformComponent* transform = (TransformComponent*)get_component_by_id(get_component_system(TRANSFORM), rendera->transform_index + i);
        transform->transform.position.y += sin(test * (i * 0.01f));

    }
}
