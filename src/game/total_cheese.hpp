#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdint.h>
//#include "../engine/message_system/message.h"
#include "../../external/math_3d.h"
#include "../osten_engine.cpp"
#include "army.hpp"

//static struct InstanceData render_ids {};

static std::vector<ArmyUnit> army_units{};

static void load_game_reasources(){
    // struct Message message{};
    // message.type = MessageType::LoadModel;
    // message.value = (void*)"assets/debug_assets/Cube.obj";

    // add_message(message);

    // message.type = MessageType::LoadTexture;
    // message.value = (void*)"assets/debug_assets/viking_room.png";

    // add_message(message);
    // const uint32_t capacity = 2000;
    // render_ids.model_index = 0;
    // render_ids.texture_index = 1;
    // render_ids.capacity = capacity +2;

    // add_message_f(MessageType::CreateRenderable, sizeof(InstanceData), (char*)&render_ids);
}

static void init_game(OstenEngine& engine){
    vkDeviceWaitIdle(engine.render_pipeline.device.virtual_device);

    create_health_system(2000, &engine.heap_stack);
    create_melee_system(2000, &engine.heap_stack);

    struct RenderAble* rendera = get_renderable(&engine.render_pipeline.model_render_data, 0, &engine.heap_stack);

    ArmyUnit unit1 = init_army_unit(rendera, {20 , 0 , 0}, 255, 40, &engine.heap_stack, 0);
    ArmyUnit unit2 = init_army_unit(rendera, {-20 , 0 , 0}, 255, 40, &engine.heap_stack, 1);

    army_units.emplace_back(unit1);
    army_units.emplace_back(unit2);
}
static double test = 0;
static int target_index = 0;

static void update_game(double delta_time, OstenEngine& engine){
    int index = 0;
    if(glfwGetKey(engine.main_window, GLFW_KEY_1) == GLFW_PRESS){
        target_index=0;
        std::string line = std::string("Index is ");
        line.push_back(target_index);
        Debug::log(line);
    }
    if(glfwGetKey(engine.main_window, GLFW_KEY_2) == GLFW_PRESS){
        target_index=1;
        std::string line = std::string("Index is ");
        line.push_back(target_index);
        Debug::log(line);
    }
    test+= delta_time;
    for (ArmyUnit& unit : army_units) {
        if(index == target_index){
            unit.target_point = engine.target_point;
        }
        move_towards(unit, engine.target_point, delta_time);
        index++;
    }
    calculate_attack(&engine.heap_stack);
    run_attack_system();
    run_health_system();
}
