#pragma once
#include <stdint.h>
#include <vector>
#include "../engine/message_system/message.h"


struct ArmyUnit{
    uint16_t unit_colliders[255];
    RenderAble* render_able;
    bool alive_units[255];//Bad
    float move_speed = 5.f;
};

static inline ArmyUnit init_army_unit(struct RenderAble* render_able, vec3_t start_point, uint8_t amount, uint8_t row_size, HeapStack* heap_stack){
    uint8_t row = 0;
    ArmyUnit unit{};
    unit.render_able = render_able;
    for(uint8_t i = 0; i < amount; i++){
        uint16_t transform_index = UINT16_MAX;
        //uint16_t entity_id = queue_enity_creation();
        struct Entity entity {};

        transform_index = add_transform();
        ((uint16_t*)get_at_index(heap_stack, render_able->transform_index))[i] = transform_index;
        unit.unit_colliders[i] = add_collider(transform_index, 0);

        //uint16_t t = ((uint16_t*)get_at_index(heap_stack, render_able->transform_index))[i];

        struct TempID transform_comp{
            (uint16_t)(transform_index),
            (uint16_t)(TRANSFORM)
        };
        struct TempID render{
            (uint16_t)(add_render_component(0, transform_index)),
            (uint16_t)(RENDER)
        };
        struct TempID collider{
            (uint16_t)(unit.unit_colliders[i]),
            (uint16_t)(COLLIDER)
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
        add_component(entity, transform_comp);
        add_component(entity, render);
        add_component(entity, collider);
        entities_to_create.emplace_back(entity);
    }
    return unit;
}

static inline void move_towards(ArmyUnit& unit, vec3_t target_position, double delta_time){
    if(delta_time <= 0) return;
    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    ComponentSystem* collider_system = get_component_system(COLLIDER);
    SimpleColliderComp* colliders = (struct SimpleColliderComp*)get_component_by_id(collider_system, unit.unit_colliders[0]);

    float minimum_distance = 4;
    float seperation_strength = 1;
    for (uint8_t i = 0; i < 255; i++) {
        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(transform_system, colliders[unit.unit_colliders[i]].transform_id);

        vec3_t current = transform->transform.position;
        vec3_t target = v3_sub(target_position, current);
        vec3_t desired = v3_norm(target);

        vec3_t separation = {};

        for (uint8_t x = 0; x < colliders[i].collision_amount; x++){
            TransformComponent* other = (struct TransformComponent*)get_component_by_id(transform_system, colliders->nearby_colliders[x]);

            vec3_t other_pos = other->transform.position;

            vec3_t diff = v3_sub(current, other_pos);
            float dist = v3_length(diff);

            if (dist > 0 && dist < minimum_distance) {
                // stronger push when closer
                vec3_t push = v3_muls(v3_norm(diff), (minimum_distance - dist) / minimum_distance);
                separation = v3_add(separation, push);
            }
        }

        separation = v3_muls(separation, seperation_strength);

        vec3_t final_dir = v3_add(desired, separation);
        final_dir = v3_norm(final_dir);
        final_dir.z = 0;
        float move_speed = unit.move_speed * delta_time;
        transform->transform.position = v3_move_towards(current, v3_add(current, v3_muls(final_dir, move_speed)), move_speed);
    }
}
