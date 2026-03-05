#pragma once
#include <stdint.h>
#include <vector>
#include "../engine/message_system/message.h"
#include "../renderer/terrain.h"


struct ArmyUnit{
    uint16_t unit_colliders[255];
    RenderAble* render_able;
    //bool alive_units[255];//Bad
    float move_speed = 5.f;
    uint8_t amount = 0;
    vec3_t target_point;
};

static inline ArmyUnit init_army_unit(struct RenderAble* render_able, vec3_t start_point, uint8_t amount, uint8_t row_size, HeapStack* heap_stack, uint8_t team_id){
    uint8_t row = 0;
    ArmyUnit unit{};
    unit.render_able = render_able;
    unit.amount = amount;
    for(uint8_t i = 0; i < amount; i++){
        uint16_t transform_index = UINT16_MAX;
        //uint16_t entity_id = queue_enity_creation();
        struct Entity entity {};

        transform_index = add_transform();
        ((uint16_t*)get_at_index(heap_stack, render_able->transform_index))[render_able->instance_amount] = transform_index;
        uint16_t collider_index = add_collider(transform_index, 0);
        unit.unit_colliders[i] = collider_index;

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
            (uint16_t)(collider_index),
            (uint16_t)(COLLIDER)
        };
        struct TempID health{
            (uint16_t)(add_health_comp(team_id, 0)),
            (uint16_t)(HEALTH)
        };
        struct TempID melee{
            (uint16_t)(add_melee_comp(0)),
            (uint16_t)(MELEE)
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
        add_component(entity, health);
        add_component(entity, melee);
        entities_to_create.emplace_back(entity);
    }
    return unit;
}

static inline void move_towards(ArmyUnit& unit, vec3_t target_position, double delta_time, Terrain* test_t){
    if(delta_time <= 0) return;
    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    ComponentSystem* collider_system = get_component_system(COLLIDER);

    SimpleColliderComp* colliders = (struct SimpleColliderComp*)get_component_by_id(collider_system, 0);

    vec2_t target_pos = {unit.target_point.x, unit.target_point.y};
    float minimum_distance = 5;
    float seperation_strength = 3;
    for (uint8_t i = 0; i < unit.amount; i++) {
        uint16_t transform_id = colliders[unit.unit_colliders[i]].transform_id;

        assert(colliders[unit.unit_colliders[i]].transform_id != 0);

        TransformComponent* transform = (struct TransformComponent*)get_component_by_id(transform_system, transform_id);
        vec2_t current = {};
        current.x = transform->transform.position.x;
        current.y = transform->transform.position.y;

        transform->transform.position.z = sample_terrain_height_interpolated(test_t, current.x, current.y);

        vec2_t target = {target_pos.x - current.x, target_pos.y - current.y};

        //vec3_t target = v3_sub(target_position, current);
        vec2_t desired = v2_norm(target);

        vec2_t separation = {};

        for (uint8_t x = 0; x < colliders[unit.unit_colliders[i]].collision_amount; x++){
            assert(colliders[unit.unit_colliders[i]].transform_id != 0);
            TransformComponent* other = (struct TransformComponent*)get_component_by_id(transform_system, colliders[unit.unit_colliders[i]].nearby_colliders[x]);

            vec2_t other_pos = { other->transform.position.x, other->transform.position.y };

            //vec3_t diff = v3_sub(current, other_pos);

            vec2_t diff = { current.x - other_pos.x, current.y - other_pos.y };

            float dist = v2_length(diff);

            if (dist > 0 && dist < minimum_distance) {
                // stronger push when closer
                vec2_t push = v2_muls(v2_norm(diff), (minimum_distance - dist) / minimum_distance);
                //separation = v3_add(separation, push);
                separation = { separation.x + push.x, separation.y + push.y };
            }
        }

        separation = v2_muls(separation, seperation_strength);

        //vec3_t final_dir = v3_add(desired, separation);
        vec2_t final_dir = {desired.x + separation.x, desired.y + separation.y};
        final_dir = v2_norm(final_dir);
        float move_speed = unit.move_speed * delta_time;

        vec3_t move = {final_dir.x * move_speed, final_dir.y * move_speed, 0};

        transform->transform.position = v3_add(transform->transform.position, move); //v3_move_towards(current, v3_add(current, v3_muls(final_dir, move_speed)), move_speed);
    }
}


static inline void calculate_attack(struct HeapStack* heap_stack){
    ComponentSystem* collider_system = get_component_system(COLLIDER);
    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    ComponentSystem* health_system = get_component_system(HEALTH);
    ComponentSystem* melee_system = get_component_system(MELEE);

    SimpleColliderComp* colliders = (struct SimpleColliderComp*)get_component_by_id(collider_system, 0);
    HealthComponent* health_comps = (struct HealthComponent*)get_component_by_id(health_system, 0);
    MeleeComponent* melee_comps = (struct MeleeComponent*)get_component_by_id(melee_system, 0);
    TransformComponent* transforms = (struct TransformComponent*)get_component_by_id(transform_system, 0);

    Entity* entities = get_all_entities().data();

    for(int i = 0; i < collider_system->amount; i++){
        SimpleColliderComp& collider = colliders[i];
        Transform my_transform = transforms[collider.transform_id].transform;
        //vec2_t my_pos = {my_transform.position.x, my_transform.position.y};
        uint16_t health_index = {};
        uint16_t attack_index = {};
        if(!has_component(entities[collider.entity_id], HEALTH, &health_index)){
            continue;
        }
        if(!has_component(entities[collider.entity_id], MELEE, &attack_index)){
            continue;
        }

        uint16_t team = health_comps[health_index].team_id;

        for (int x = 0; x < collider.collision_amount; x++) {
            //SimpleColliderComp nearby_collider = colliders[collider.nearby_colliders[x]];
            TransformComponent other_transform = transforms[collider.nearby_colliders[x]];

            vec3_t diff = v3_sub(my_transform.position, other_transform.transform.position);
            float dist_sq = v3_dot(diff, diff);

            if(dist_sq > 100 || dist_sq <= 0.01f){
                continue;
            }
            Entity other_entity = entities[other_transform.entity_id];

            uint16_t other_health_index = {};

            if(has_component(other_entity, HEALTH, &other_health_index)){
                HealthComponent other_health = health_comps[other_health_index];
                if(other_health.team_id != team){
                    melee_comps[attack_index].nearby_enemy_health_id = other_health_index;
                }
            }
        }
    }
}

static inline bool is_unit_alive(ArmyUnit* unit){
    ComponentSystem* health_system = get_component_system(HEALTH);
    HealthComponent* health_comps = (HealthComponent*)get_component_by_id(health_system, 0);

    ComponentSystem* collider_system = get_component_system(COLLIDER);
    SimpleColliderComp* collider_comps = (SimpleColliderComp*)get_component_by_id(collider_system, 0);

    for(uint8_t i = 0; i < unit->amount; i++){
        uint16_t health_id = 0;
        uint16_t unit_id = collider_comps[unit->unit_colliders[i]].entity_id;
        if(has_component(entities[unit_id], HEALTH, &health_id)){
            int16_t health = health_comps[health_id].health;
            if(health> 0) {
                return true;
            }
        }
    }
    return false;
}


static inline vec3_t get_position(ArmyUnit* unit){
    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    ComponentSystem* collider_system = get_component_system(COLLIDER);

    SimpleColliderComp* colliders = 0;

    for(int i = 0; i < 255; i++){
        colliders = (SimpleColliderComp*)get_component_by_id(collider_system, unit->unit_colliders[i]);
        TransformComponent* transforms = (TransformComponent*)get_component_by_id(transform_system, colliders->transform_id);
        if(transforms->transform.position.x < 1000){

            return transforms->transform.position;
        }
    }
    return {};
}
