#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdio.h>
#include <stdint.h>
//#include "../engine/message_system/message.h"
#include "../../external/math_3d.h"
#include "../osten_engine.cpp"
#include "GLFW/glfw3.h"
#include "army.hpp"
#include "../renderer/terrain.h"

//static struct InstanceData render_ids {};

struct GameData{
    bool* paused_state;
    Terrain terrain;
    void (*game_code)(OstenEngine*, GameData*) = nullptr;
};

static std::vector<ArmyUnit> army_units{};

static uint32_t valid_units = 0;

//static uint32_t selected_unit[9];

static int target_index = 0;

static void update_game(OstenEngine* engine, GameData* data){
    int index = 0;
    for(uint32_t i = 0; i < valid_units; i++){
        int key = GLFW_KEY_1 + i;
        if(glfwGetKey(engine->main_window, key) == GLFW_PRESS){
            target_index= i;
        }
    }
    for (ArmyUnit& unit : army_units) {
        if(index == target_index){
            unit.target_point = engine->target_point;
        }
        move_towards(unit, engine->target_point, engine->delta_time, &data->terrain);
        index++;
    }
    calculate_attack(&engine->heap_stack);
    run_attack_system();
    run_health_system();

    uint16_t friendly_units = 0;
    for(uint32_t i = 0; i < valid_units; i++){
        if(is_unit_alive(&army_units[i])){
            friendly_units++;
        }
    }
    uint16_t enemy_units = 0;
    for(uint32_t i = valid_units; i < army_units.size(); i++){
        if(is_unit_alive(&army_units[i])){
            enemy_units++;
            if(friendly_units > 0){
                army_units[i].target_point = get_position(&army_units[friendly_units-1]);
            }
        }
    }
    char test[255];
    sprintf(test, "Friendly Units Alive %i", friendly_units);
    ImGui::Text("%s", test);

    char enemy[255];
    sprintf(enemy, "Enemy Units Alive %i", enemy_units);
    ImGui::Text("%s", enemy);

    if(friendly_units <= 0){
        ImGui::Text("You Lost");
    }
    if(enemy_units <= 0){
        ImGui::Text("You Win");
    }

}

static void init_game(OstenEngine* engine, GameData* data){
    vkDeviceWaitIdle(engine->render_pipeline.device.virtual_device);

    struct RenderAble* rendera = get_renderable(&engine->render_pipeline.model_render_data, 0, &engine->heap_stack);
    struct RenderAble* render2a = get_renderable(&engine->render_pipeline.model_render_data, 1, &engine->heap_stack);
    uint16_t enemy_units = 2;
    uint16_t friendly_units = 2;
    valid_units = friendly_units;
    for(int i = 0; i < friendly_units; i++){
        ArmyUnit unit1 = init_army_unit(rendera, {250 , 50 + (float)i * 50 , 0}, 255, 10, &engine->heap_stack, 0);
        army_units.emplace_back(unit1);
    }
    for(int i = 0; i < enemy_units; i++){
        ArmyUnit unit1 = init_army_unit(render2a, {-250 , -50 + (float)i * 50 , 0}, 255, 10, &engine->heap_stack, 1);
        army_units.emplace_back(unit1);
    }

    std::string test = std::to_string((friendly_units + enemy_units) * 255);
    Debug::log(test);

    //tr->transform.rotation = {-1.6, -1.6, 0};
    engine->open_window = false;
    data->game_code = update_game;
}


static void menu_state(OstenEngine* engine, GameData* data){
    ImGui::Begin("Game UI", &engine->open_window);
    if(ImGui::Button("Start")){

        uint16_t transform_index = UINT16_MAX;

        struct Entity entity {};

        transform_index = add_transform();

        RenderAble* render = get_renderable(&engine->render_pipeline.model_render_data, 2, &engine->heap_stack);

        ((uint16_t*)get_at_index(&engine->heap_stack, render->transform_index))[render->instance_amount] = transform_index;

        ComponentSystem* transfomr = get_component_system(TRANSFORM);

        TransformComponent* tr = (TransformComponent*)get_component_by_id(transfomr, transform_index);

        tr->transform.position = {-((float)data->terrain.width/2), -((float)data->terrain.height/2), 0};

        data->terrain.pos_x = &tr->transform.position.x;
         data->terrain.pos_y = &tr->transform.position.y;

        struct ComponentID transform_comp{
            (uint16_t)(transform_index),
            (uint16_t)(TRANSFORM)
        };
        struct ComponentID render2{
            (uint16_t)(add_render_component(0, transform_index)),
            (uint16_t)(RENDER)
        };

        add_component(entity, transform_comp);
        add_component(entity, render2);
        entities_to_create.emplace_back(entity);
        render->instance_amount++;
        data->game_code = init_game;
    }
    ImGui::End();
}

static void load_game_resources(OstenEngine* engine, GameData* data){

    create_terrain(1000, 1000, &data->terrain, &engine->heap_stack);

    create_terrain_mesh(data->terrain, &engine->render_pipeline);

    //free_arena(&engine->heap_stack, free_index);

    struct InstanceData render_ids = {};

    render_ids.model_index = loaded_models.size()-1;
    render_ids.texture_index = 1;
    render_ids.capacity = 2;

    add_message_f(MessageType::CreateRenderable, sizeof(InstanceData), (char*)&render_ids);

    data->game_code = menu_state;
}
