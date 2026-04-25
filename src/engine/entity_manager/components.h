#pragma once
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "../transform.h"
#include "../../additional_things/arena.h"

#define CAMERA 0
#define TRANSFORM 1
#define RENDER 2
#define COLLIDER 3
#define HEALTH 4
#define MELEE 5

/**
    Base version of a Component that is used as the cast when checking what the component it is.
*/
typedef struct Component{
    const uint16_t id;
    uint16_t entity_id;
} Component;

/**
    Bad representation of the transform component as it could be formated in a better way
*/
typedef struct TransformComponent{
    const uint16_t id;
    uint16_t entity_id;
    Transform transform;
} TransformComponent;

/**
    This is the component representation of the renderable and it contains the index of the render instance and transform index
*/
typedef struct RenderComponent{
    const uint16_t id;
    uint16_t entity_id;
    uint16_t transform_id;
    uint32_t instance_id;
} RenderComponent;

/**
    This is the component representation of the Camera
*/
typedef struct CameraComponent{
    const uint16_t id;
    uint16_t entity_id;
    uint16_t transform_id;
    float field_of_view;
} CameraComponent;

/**
    This is the component representation of the collider and all it does is have a range and a statically allocated array with a max capacity of 42
*/
typedef struct SimpleColliderComp{
    const uint16_t id;
    uint16_t entity_id;
    uint16_t transform_id;
    uint16_t collision_amount;
    float collision_range;
    uint16_t nearby_colliders[42];//Bad and can cause issues but to little time to fix
} SimpleColliderComp;

/**
    This is the component representation of the health of a entity(it is bad)
*/
typedef struct HealthComponent{
    const uint16_t id;
    uint16_t entity_id;
    int16_t health;
    uint16_t damage_taken;
    uint16_t team_id;
} HealthComponent;

/**
    This is the component representation of the Melee attack
*/
typedef struct MeleeComponent{
    const uint16_t id;
    uint16_t entity_id;
    uint16_t damage;
    uint16_t nearby_enemy_health_id;
    float attack_cooldown;
} MeleeComponent;

/**
    This is System that basically works as a array that is used by the systems that need to access systems
*/
typedef struct ComponentSystem{
    HeapStack* memory_arena;
    size_t components;
    uint16_t amount;
    uint16_t capacity;
    uint8_t type;
} ComponentSystem;

static ComponentSystem cameras = {};
static ComponentSystem transforms = {};
static ComponentSystem render_components = {};
static ComponentSystem simple_colliders = {};
static ComponentSystem health_system = {};
static ComponentSystem melee_system = {};


//I don't like
static inline uint16_t get_component_size_by_type(uint16_t type){
    switch (type){
    case TRANSFORM:
        return sizeof(TransformComponent);
    case RENDER:
        return sizeof(RenderComponent);
    case CAMERA:
        return sizeof(CameraComponent);
    case COLLIDER:
        return sizeof(SimpleColliderComp);
    case HEALTH:
        return sizeof(HealthComponent);
    case MELEE:
        return sizeof(MeleeComponent);
    default:
        return 0;
    }
}

static inline void* get_component_by_id(ComponentSystem* component_system, uint16_t id){
    if(id > component_system->amount) return 0;
    uint8_t* comp = (uint8_t*)get_at_index(component_system->memory_arena, component_system->components);
    uint16_t size = get_component_size_by_type(component_system->type);
    uint32_t size_offset = size * id;
    comp += size_offset;
    return comp;
}

static inline void* remove_component_by_id(ComponentSystem* component_system, uint16_t id){
    assert(id <= component_system->amount);
    // uint8_t* comp = (uint8_t*)get_at_index(component_system->memory_arena, component_system->components);
    // uint8_t* replace = (uint8_t*)get_at_index(component_system->memory_arena, component_system->components);
    // uint16_t size = get_component_size_by_type(component_system->type);
    // uint32_t size_offset = size * id;
    // comp += size_offset;

    // replace += ((component_system->amount-1) * size);

    // //memcpy(comp, replace, size);

    // component_system->amount--;
    // return comp;
    return 0;
}


static inline ComponentSystem* get_component_system(uint8_t system_id){
    switch (system_id){
    case CAMERA:
        return &cameras;
    case TRANSFORM:
        return &transforms;
    case RENDER:
        return &render_components;
    case COLLIDER:
        return &simple_colliders;
    case HEALTH:
        return &health_system;
    case MELEE:
        return &melee_system;
    default:
        return 0;
    }
}

static inline uint16_t add_render_component(uint16_t descriptor_index, uint16_t transform_index){
    ComponentSystem* component_sys = get_component_system(RENDER);
    RenderComponent* comp = (RenderComponent*)(get_at_index(component_sys->memory_arena,component_sys->components));
    // uint16_t size = get_component_size_by_type(RENDER);
    uint32_t size_offset = component_sys->amount;
    comp += size_offset;
    component_sys->amount++;
    comp->instance_id = descriptor_index;
    comp->transform_id = transform_index;
    return component_sys->amount-1;
}

static inline void create_transform_system(uint16_t transform_amount, HeapStack* memory_arena){
    ComponentSystem* component_system = get_component_system(TRANSFORM);
    component_system->components = arena_alloc_memory(memory_arena, sizeof(TransformComponent) * transform_amount);
    component_system->memory_arena = memory_arena;
    component_system->capacity = transform_amount;
    component_system->type = TRANSFORM;
    TransformComponent* comp = (TransformComponent*)get_at_index(component_system->memory_arena, component_system->components);

    for (size_t i = 0; i < transform_amount; i++){
        comp->transform = (Transform){};
        comp->transform.scale = (vec3_t){1, 1, 1};
        comp++;
    }
}

static inline void create_render_component_system(uint16_t render_amount, HeapStack* memory_arena){
    ComponentSystem* component_system = get_component_system(RENDER);
    component_system->memory_arena = memory_arena;
    component_system->components = arena_alloc_memory(memory_arena, sizeof(RenderComponent) * render_amount);
    component_system->type = RENDER;
    component_system->capacity = render_amount;

    RenderComponent* comp = (RenderComponent*)get_at_index(component_system->memory_arena, component_system->components);
    for (size_t i = 0; i < render_amount; i++){
        comp->transform_id = -1;
        comp->instance_id = 0;
        comp++;
    }
}

static inline void create_camera_system(uint8_t camera_amount, HeapStack* memory_arena){
    ComponentSystem* component_system = get_component_system(CAMERA);

    component_system->memory_arena = memory_arena;
    component_system->components = arena_alloc_memory(memory_arena, sizeof(CameraComponent) * camera_amount);
    component_system->amount = 0;
    component_system->type = CAMERA;
    component_system->capacity = camera_amount;

    CameraComponent* comp = (CameraComponent*)get_at_index(component_system->memory_arena, component_system->components);
    for (size_t i = 0; i < camera_amount; i++){
        comp->field_of_view = 45.f;
        comp++;
    }
}

static inline void create_collider_system(uint16_t collider_amount, HeapStack* memory_arena){
    ComponentSystem* component_sys = get_component_system(COLLIDER);

    component_sys->memory_arena = memory_arena;
    component_sys->components = arena_alloc_memory(memory_arena, sizeof(SimpleColliderComp) * collider_amount);
    component_sys->capacity = collider_amount;
    component_sys->amount = 0;
    component_sys->type = COLLIDER;
}

static inline void create_health_system(uint16_t health_capacity, HeapStack* memory_arena){
    ComponentSystem* component_sys = get_component_system(HEALTH);

    component_sys->memory_arena = memory_arena;
    component_sys->components = arena_alloc_memory(memory_arena, sizeof(HealthComponent) * health_capacity);
    component_sys->capacity = health_capacity;
    component_sys->amount = 0;
    component_sys->type = HEALTH;
}

static inline void create_melee_system(uint16_t melee_capacity, HeapStack* memory_arena){
    ComponentSystem* component_sys = get_component_system(MELEE);

    component_sys->memory_arena = memory_arena;
    component_sys->components = arena_alloc_memory(memory_arena, sizeof(MeleeComponent) * melee_capacity);
    component_sys->capacity = melee_capacity;
    component_sys->amount = 0;
    component_sys->type = MELEE;
}

static inline uint16_t add_camera(uint16_t transform_index){
    ComponentSystem* component_sys = get_component_system(CAMERA);

    CameraComponent* comp = (CameraComponent*)get_at_index(component_sys->memory_arena, component_sys->components);
    comp += component_sys->amount;
    comp->transform_id = transform_index;

    TransformComponent* transform = (TransformComponent*)get_component_by_id(&transforms, comp->transform_id);
    transform->transform.position.x = 20;
    transform->transform.rotation.y = 0;
    transform->transform.position.z = 5;

    component_sys->amount++;

    return component_sys->amount-1;
}

static inline uint16_t add_transform(){
    ComponentSystem* component_sys = get_component_system(TRANSFORM);
    // TransformComponent* comp = (TransformComponent*)get_at_index(component_sys->memory_arena, component_sys->components);
    // comp += component_sys->amount;
    component_sys->amount++;
    return component_sys->amount-1;
}

static inline uint16_t add_collider(uint16_t transform_index, uint16_t entity){
    ComponentSystem* component_sys = get_component_system(COLLIDER);
    SimpleColliderComp* comp = (SimpleColliderComp*)get_at_index(component_sys->memory_arena, component_sys->components);
    comp += component_sys->amount;
    comp->collision_range = 5;
    comp->transform_id = transform_index;
    component_sys->amount++;
    return component_sys->amount-1;
}

static inline uint16_t add_health_comp(uint8_t team,uint16_t entity){
    ComponentSystem* component_sys = get_component_system(HEALTH);
    HealthComponent* comp = (HealthComponent*)get_at_index(component_sys->memory_arena, component_sys->components);
    comp += component_sys->amount;
    comp->health = 100;
    comp->team_id = team;
    component_sys->amount++;
    return component_sys->amount-1;
}

static inline uint16_t add_melee_comp(uint16_t entity){
    ComponentSystem* component_sys = get_component_system(MELEE);
    MeleeComponent* comp = (MeleeComponent*)get_at_index(component_sys->memory_arena, component_sys->components);
    comp += component_sys->amount;
    comp->entity_id = entity;
    comp->damage = 1;
    comp->nearby_enemy_health_id = UINT16_MAX;
    component_sys->amount++;
    return component_sys->amount-1;
}

static inline void calculate_colliders(){
    ComponentSystem* collider_system = get_component_system(COLLIDER);
    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    SimpleColliderComp* colliders = (SimpleColliderComp*)get_at_index(collider_system->memory_arena, collider_system->components);
    TransformComponent* transforms = (TransformComponent*)get_at_index(transform_system->memory_arena, transform_system->components);
    const uint32_t collider_capacity = sizeof(colliders[0].nearby_colliders) / sizeof(colliders[0].nearby_colliders[0]);

    for(int x = 0; x < collider_system->amount; x++){
        SimpleColliderComp* my_collider = &colliders[x];
        my_collider->collision_amount = 0;
        Transform self = transforms[my_collider->transform_id].transform;

        for(int y = 0; y < collider_system->amount; y++){
            uint16_t other_transform_id = colliders[y].transform_id;
            Transform other = transforms[other_transform_id].transform;
            vec3_t diff = v3_sub(self.position, other.position);
            float dist_sq = v3_dot(diff, diff);

            float range = my_collider->collision_range;
            if (dist_sq < range * range && dist_sq > 0.1f){
            //if(length < colliders[x].collision_range && length > 0.1f){

                if(my_collider->collision_amount >= collider_capacity){
                    //Debug::log("You had to many collisions");
                    continue;
                    assert(false && "You had to many collisions");
                    //break;
                }
                if(other_transform_id == 0){

                    assert(false && "You somehow tried to collide with the camera?");
                    //break;
                }
                my_collider->nearby_colliders[my_collider->collision_amount] = other_transform_id;
                my_collider->collision_amount++;
            }
        }
    }
}


static inline void run_attack_system(){
    ComponentSystem* health_system = get_component_system(HEALTH);
    ComponentSystem* melee_system = get_component_system(MELEE);
    MeleeComponent* melee_comp = (MeleeComponent*)get_component_by_id(melee_system, 0);
    HealthComponent* health_comps = (HealthComponent*)get_component_by_id(health_system, 0);


    for (int i = 0; i < melee_system->amount; i++) {
        if(!health_comps) continue;
        uint16_t enemy_health_id = melee_comp[i].nearby_enemy_health_id;
        if(enemy_health_id == UINT16_MAX) continue;
        health_comps[enemy_health_id].damage_taken += melee_comp[i].damage;
        melee_comp[i].nearby_enemy_health_id = UINT16_MAX;
    }
}
