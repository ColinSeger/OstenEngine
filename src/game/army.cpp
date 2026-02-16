#pragma once
#include <cstdint>
#include <vector>
#include "../engine/message_system/message.h"
//#include "../osten_engine.cpp"
//#include "../renderer/render_pipeline.cpp"
//#include "../engine/entity_manager/entity_manager.cpp"


struct ArmyUnit{
    uint16_t units_transform[255];
    bool alive_units[255];//Bad
    float move_speed = 1.f;
};

ArmyUnit init_army_unit(struct RenderAble* render_able, vec3_t start_point, uint8_t amount, uint8_t row_size){
    uint8_t row = 0;
    ArmyUnit unit{};

    struct Entity entity {};

    for(uint8_t i = 0; i < amount; i++){

        unit.units_transform[i] = render_able->transform_index + render_able->instance_amount;

        struct TempID render{
            (uint16_t)(add_render_component(0, unit.units_transform[i])),
            (uint16_t)(RENDER)
        };

        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(get_component_system(TRANSFORM), unit.units_transform[i]);
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
        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(get_component_system(TRANSFORM), unit.units_transform[i]);
        transform->transform.rotation.y += 0.01f;
    }
}

void move_towards(ArmyUnit& unit, vec3_t position, double delta_time){
    for (uint8_t i = 0; i < 255; i++) {
        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(get_component_system(TRANSFORM), unit.units_transform[i]);
        vec3_t current = transform->transform.position;
        vec3_t direction = v3_sub(position, current);
        float lenght = v3_length(direction);

        vec3_t l = v3_div(direction, {lenght, lenght, lenght});


        l = v3_mul({unit.move_speed, unit.move_speed, unit.move_speed}, l);

        transform->transform.position = v3_add(current, l);
    }
}
