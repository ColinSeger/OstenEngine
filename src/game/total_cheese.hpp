#pragma once
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdint.h>
//#include "../engine/message_system/message.h"
#include "../../external/math_3d.h"
#include "../osten_engine.cpp"
#include "army.hpp"
#include "../renderer/terrain.h"

//static struct InstanceData render_ids {};

static std::vector<ArmyUnit> army_units{};

static void load_game_reasources(OstenEngine& engine){
    // return;
    // Terrain terrain = {};

    // create_terrain(10, 10, &terrain);

    // create_terrain_mesh(terrain, &engine.render_pipeline);
    // struct InstanceData render_ids = {};

    // render_ids.model_index = loaded_models.size()-1;
    // render_ids.texture_index = 0;
    // render_ids.capacity = 2;

    // add_message_f(MessageType::CreateRenderable, sizeof(InstanceData), (char*)&render_ids);
}

static void init_game(OstenEngine& engine){
    vkDeviceWaitIdle(engine.render_pipeline.device.virtual_device);

    create_health_system(10000, &engine.heap_stack);
    create_melee_system(10000, &engine.heap_stack);

    struct RenderAble* rendera = get_renderable(&engine.render_pipeline.model_render_data, 0, &engine.heap_stack);
    struct RenderAble* render2a = get_renderable(&engine.render_pipeline.model_render_data, 1, &engine.heap_stack);
    uint16_t unit_amount = 1;
    for(int i = 0; i < unit_amount; i++){
        ArmyUnit unit1 = init_army_unit(rendera, {50 + (float)i * 50 , 0 , 0}, 255, 10, &engine.heap_stack, 0);
        army_units.emplace_back(unit1);
    }
    for(int i = 0; i < unit_amount; i++){
        ArmyUnit unit1 = init_army_unit(render2a, {-50 + (float)i * 50 , 0 , 0}, 255, 10, &engine.heap_stack, 1);
        army_units.emplace_back(unit1);
    }

    std::string test = std::to_string((2* unit_amount) * 255);
    Debug::log(test);
}

static double test = 0;
static int target_index = 0;

static void update_game(double delta_time, OstenEngine& engine, Terrain terrain){
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
        move_towards(unit, engine.target_point, delta_time, terrain);
        index++;
    }
    calculate_attack(&engine.heap_stack);
    run_attack_system();
    run_health_system();
}
