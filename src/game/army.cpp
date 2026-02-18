#pragma once
#include <cstdint>
#include <vector>
#include "../engine/message_system/message.h"
//#include "../osten_engine.cpp"
//#include "../renderer/render_pipeline.cpp"
//#include "../engine/entity_manager/entity_manager.cpp"


struct ArmyUnit{
    uint16_t unit_transform[255];
    RenderAble* render_able;
    bool alive_units[255];//Bad
    float move_speed = 5.f;
};

ArmyUnit init_army_unit(struct RenderAble* render_able, vec3_t start_point, uint8_t amount, uint8_t row_size){
    uint8_t row = 0;
    ArmyUnit unit{};
    unit.render_able = render_able;
    for(uint8_t i = 0; i < amount; i++){

        uint16_t transform_index = render_able->transform_index + render_able->instance_amount;
        unit.unit_transform[i] = transform_index;

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
    ComponentSystem* system = get_component_system(TRANSFORM);
    for (uint8_t i = 0; i < 255; i++) {
        //uint16_t id = get_component_id(unit.unit_transform[i], TRANSFORM);
        //if(id == UINT16_MAX) continue;
        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(system, unit.unit_transform[i]);
        transform->transform.rotation.y += 0.01f;
    }
}

void move_towards(ArmyUnit& unit, vec3_t target_position, double delta_time, uint16_t nearby_transform_indexes[255]){
    if(delta_time <= 0) return;
    ComponentSystem* system = get_component_system(TRANSFORM);

    float minimum_distance = 4;
    float seperation_strenght = 1;


    for (uint8_t i = 0; i < 255; i++) {

        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(system, unit.unit_transform[i]);
        vec3_t current = transform->transform.position;
        vec3_t target = v3_sub(target_position, current);
        vec3_t desired = v3_norm(target);

        vec3_t separation = {};

        //vec3_t result = v3_move_towards(current, desired, delta_time * unit.move_speed * 10);

        for (uint8_t x = 0; x < 255; x++){
            if(nearby_transform_indexes[x] == 0) break;
            TransformComponent* other = (struct TransformComponent*)get_component_by_id(system, nearby_transform_indexes[x]);

            vec3_t other_pos = other->transform.position;

            vec3_t diff = v3_sub(current, other_pos);
            float dist = v3_length(diff);

            if (dist > 0 && dist < minimum_distance) {
                // stronger push when closer
                vec3_t push = v3_muls(v3_norm(diff), (minimum_distance - dist) / minimum_distance);
                separation = v3_add(separation, push);
            }
        }

        separation = v3_muls(separation, seperation_strenght);

        vec3_t final_dir = v3_add(desired, separation);
        final_dir = v3_norm(final_dir);
        float move_speed = unit.move_speed * delta_time;
        transform->transform.position = v3_move_towards(current, v3_add(current, v3_muls(final_dir, move_speed)), move_speed);
    }
}

uint8_t get_units_in_range(ArmyUnit* army_units, uint16_t unit_amount, vec3_t target, float range, uint16_t* result, uint8_t capacity){
    ComponentSystem* transforms = get_component_system(TRANSFORM);
    uint8_t amount = 0;
    for(uint16_t x = 0; x < unit_amount; x++){
        for(uint16_t i = 0; i < 255; i++){
            //uint16_t id = get_component_id(army_units[x].units[i], TRANSFORM);
            //if(id == UINT16_MAX) continue;
            Transform& transform1 = ((struct TransformComponent*)get_component_by_id(transforms, army_units->unit_transform[i]))->transform;

            vec3_t distance = v3_sub(transform1.position, target);

            float vec_lenght = v3_length(distance);
            if(vec_lenght < range){
                result[amount] = army_units->unit_transform[i];
                amount++;
                if(amount >= capacity) return amount;
            }
        }
    }
    return amount;
}
