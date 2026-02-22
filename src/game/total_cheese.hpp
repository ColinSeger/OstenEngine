#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdint.h>
#include "../engine/message_system/message.h"
#include "../../external/math_3d.h"
#include "../osten_engine.cpp"
#include "army.hpp"

static struct InstanceData render_ids {};

static std::vector<ArmyUnit> army_units{};

static void load_game_reasources(){
    struct Message message{};
    message.type = MessageType::LoadModel;
    message.value = (void*)"assets/debug_assets/Cube.obj";

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

    struct RenderAble* rendera = get_renderable(engine.render_pipeline.model_render_data, 0, &engine.heap_stack);

    ArmyUnit unit1 = init_army_unit(rendera, {20 , 0 , 0}, 255, 40, &engine.heap_stack);
    ArmyUnit unit2 = init_army_unit(rendera, {-20 , 0 , 0}, 255, 40, &engine.heap_stack);

    army_units.emplace_back(unit1);
    army_units.emplace_back(unit2);
}
static double test = 0;
static void update_game(double delta_time, OstenEngine& engine){
    test+= delta_time;
    for (ArmyUnit& unit : army_units) {
        //test_army(unit);
        move_towards(unit, engine.target_point, delta_time);
    }
}
