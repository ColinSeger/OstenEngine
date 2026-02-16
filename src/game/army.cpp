#pragma once
#include <cstdint>
#include <vector>
#include "../engine/message_system/message.h"
//#include "../osten_engine.cpp"
//#include "../renderer/render_pipeline.cpp"
//#include "../engine/entity_manager/entity_manager.cpp"


struct ArmyUnit{
    Entity units[255];
    bool alive_units[255];//Bad
    float move_speed = 1.f;
};

ArmyUnit init_army_unit(struct RenderAble* render_able, vec3_t start_point, uint8_t amount, uint8_t row_size){
    uint8_t row = 0;
    ArmyUnit unit{};


    for(uint8_t i = 0; i < amount; i++){

        uint16_t transform_index = render_able->transform_index + render_able->instance_amount;

        struct Entity entity {};

        struct TempID render{
            (uint16_t)(add_render_component(0, transform_index)),
            (uint16_t)(RENDER)
        };

        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(get_component_system(TRANSFORM), transform_index);
        transform->transform.position = start_point;
        transform->transform.position.y += 2.f  * i;
        transform->transform.position.y -= 2    * row_size * row;
        transform->transform.position.x -= 2.f  * row;

        render_able->instance_amount++;

        if(1 == i%row_size){
            row++;
        }
        add_component(entity, render);
        entities_to_create.emplace_back(entity);
    }
    return unit;
}


void test_army(ArmyUnit& unit){
    for (uint8_t i = 0; i < 255; i++) {
        uint16_t id = get_component_id(unit.units[i], TRANSFORM);
        if(id == UINT16_MAX) continue;
        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(get_component_system(TRANSFORM), id);
        transform->transform.rotation.y += 0.01f;
    }
}

void move_towards(ArmyUnit& unit, vec3_t position, double delta_time){
    if(delta_time <= 0 || delta_time > 10) return;
    for (uint8_t i = 0; i < 255; i++) {
        uint16_t id = get_component_id(unit.units[i], TRANSFORM);
        if(id == UINT16_MAX) continue;
        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(get_component_system(TRANSFORM), id);
        vec3_t current = transform->transform.position;

        vec3_t result = v3_move_towards(current, position, delta_time * unit.move_speed * 10);

        transform->transform.position = result;
    }
}

uint8_t get_units_in_range(ArmyUnit* army_units, uint16_t unit_amount, vec3_t target, float range, Entity* result, uint8_t capacity){
    ComponentSystem* transforms = get_component_system(TRANSFORM);
    uint8_t amount = 0;
    for(uint16_t x = 0; x < unit_amount; x++){
        for(uint16_t i = 0; i < 255; i++){
            uint16_t id = get_component_id(army_units[x].units[i], TRANSFORM);
            if(id == UINT16_MAX) continue;
            Transform& transform1 = ((struct TransformComponent*)get_component_by_id(transforms, id))->transform;

            vec3_t distance = v3_sub(transform1.position, target);

            float vec_lenght = v3_length(distance);
            if(vec_lenght < range){
                result[amount] = army_units->units[i];
                amount++;
                if(amount >= capacity) return amount;
            }
        }
    }
    return amount;
}
