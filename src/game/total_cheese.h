#pragma once
#include "../engine/message_system/message.h"
#include "../../external/math_3d.h"
#include "../osten_engine.cpp"
#include <cmath>
#include <cstdint>
#include <vulkan/vulkan_core.h>

static struct InstanceData render_ids {};

static void load_game_reasources(){
    struct Message message{};
    message.type = MessageType::LoadModel;
    message.value = (void*)"assets/debug_assets/cube.bin";

    add_message(message);

    message.type = MessageType::LoadTexture;
    message.value = (void*)"assets/debug_assets/viking_room.png";

    add_message(message);
    const uint32_t capacity = 2000;
    render_ids.model_index = 0;
    render_ids.texture_index = 1;
    render_ids.capacity = capacity +2;

    message.type = MessageType::CreateRenderable;
    message.value = (void*)&render_ids;

    add_message(message);
}

static void init_game(OstenEngine& engine){
    vkDeviceWaitIdle(engine.render_pipeline.device.virtual_device);
    const uint32_t capacity = 2000;
    uint32_t row_size = 100;
    uint32_t row = 0;
    for(uint32_t i = 0; i < capacity; i++){
        struct Entity entity {};

        struct RenderAble* rendera = get_renderable(engine.render_pipeline.model_render_data, 0, engine.heap_stack);

        struct TempID render{
            (uint16_t)(add_render_component(0, rendera->transform_index + rendera->instance_amount)),
            (uint16_t)(RENDER)
        };
        rendera->instance_amount++;

        struct TransformComponent* transform = (struct TransformComponent*)get_component_by_id(get_component_system(TRANSFORM) ,rendera->transform_index + rendera->instance_amount);

        transform->transform.position.y += 2.f * rendera->instance_amount;
        transform->transform.position.y -= 2 * row_size * row;
        transform->transform.position.x -= 2.f * row;
        if(1 == i%row_size){
            row++;
        }
        add_component(entity, render);
        entities_to_create.emplace_back(entity);
    }
}
static double test = 0;
static void update_game(double delta_time, OstenEngine& engine){
    test+= delta_time;
    RenderAble* rendera = get_renderable(engine.render_pipeline.model_render_data, 0, engine.heap_stack);
    for (uint16_t i = 0; i < rendera->instance_amount-1; i++) {
        //TransformComponent* transform = (TransformComponent*)get_component_by_id(get_component_system(TRANSFORM), rendera->transform_index + i);
        //transform->transform.position.y += sin(test * (i * 0.01f));

    }
}
