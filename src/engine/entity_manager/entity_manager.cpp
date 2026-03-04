// #include "entity_manager.h"
#pragma once
#include "components.h"
#include <cstdint>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include "../../renderer/render_pipeline.cpp"
// #include "entity_system.cpp"

struct TempID{
    uint16_t index = 0;
    uint16_t type = 0;
};

struct Entity{
    TempID components[5];
    uint16_t id;
    uint16_t component_amount;
};

static inline void add_component(Entity& entity, TempID component){
    for(TempID com : entity.components){
        if(com.type == component.type) return;
    }
    entity.components[entity.component_amount] = component;
    entity.component_amount++;
}

static inline bool has_component(Entity entity, uint16_t component, uint16_t* index){
    for(TempID com : entity.components){
        if(com.type != component) continue;
        *index = com.index;
        return true;
    }
    return false;
}

namespace {
    std::unordered_map<std::string, uint32_t> entity_names;
    Entity entities[UINT16_MAX];//Not good
    uint16_t entities_amount = 0;
}
static inline std::unordered_map<std::string, uint32_t>& get_entity_names(){
    return entity_names;
}

static inline void add_entity(Entity entity, std::string name)
{
    auto contains = entity_names.find(name);
    while(contains != get_entity_names().end())
    {
        char buf[11];
        snprintf(buf, sizeof(buf), "%u", (uint32_t)entities_amount);
        for(char c : buf){
            name.push_back(c);
        }
        for (uint8_t i = 1; i < 9; i++)
        {
            contains = entity_names.find(name);
            if(contains != get_entity_names().end()){
                uint32_t index = name.size()-1;
                name[index] = (char)i + '0';
            }
            else {
                break;
            }
        }
    }

    entity.id = entities_amount;

    for(TempID component : entity.components){
        ComponentSystem* system = get_component_system(component.type);
        Component* comp = (Component*)get_component_by_id(system, component.index);
        comp->entity_id = entity.id;
    }

    entities[entities_amount] = entity;
    entities_amount++;
    entity_names[name] = entity.id;
}

static inline void remove_entity(Entity entity)
{
    //std::find(entities.begin(), entities.end(), entity);
}

static inline void remove_entity(uint32_t entity_id, struct RenderPipeline* bad_design, HeapStack* heap_stack)
{
    for(Entity& entity : entities){
        if(entity.id != entity_id) continue;
        uint16_t transform = 0;
        if(has_component(entity, TRANSFORM, &transform)){
            uint16_t render_id = 0;

            if(has_component(entity, RENDER, &render_id)){
                ComponentSystem* render_sustem = get_component_system(RENDER);
                //ComponentSystem* transform_system = get_component_system(TRANSFORM);
                RenderComponent* render = (RenderComponent*)get_component_by_id(render_sustem, render_id);
                //TransformComponent* transform_component = (TransformComponent*)get_component_by_id(transform_system, transform);

                RenderAble* r = get_renderable(&bad_design->model_render_data, render->instance_id, heap_stack);
                if(r->instance_amount > 0){
                    for(uint32_t i = 0; i < r->instance_amount; i++){
                        uint16_t* transform_index = (uint16_t*)get_at_index(heap_stack, r->transform_index);
                        // uint16_t test = transform_index[i];
                        // uint16_t test2 = transform_index[r->instance_amount-1];
                        if(transform_index[i] == transform){
                            r->instance_amount--;
                            transform_index[i] = transform_index[r->instance_amount];
                            break;
                        }
                    }
                }
            }

        }


        for(int i = 0; i < entity.component_amount; i++){
            ComponentSystem* system = get_component_system(entity.components[i].type);


            remove_component_by_id(system, entity.components[i].index);
        }

        entity.component_amount = 0;
        entities_amount--;

        break;
    }

    // for (size_t i = 0; i < entities.size(); i++)
    // {
    //     if(entities[i].id == entity_id){
    //         entities.erase(entities.begin() + i);
    //         return;
    //     }
    // }
}

static inline uint32_t get_entity_amount()
{
    return entities_amount;
}

static inline void rename_entity(std::string current_name, std::string new_name)
{
    auto contains = entity_names.find(current_name);
    if(contains != get_entity_names().end()){
        uint32_t id = entity_names[current_name];
        entity_names.erase(current_name);
        entity_names[new_name] = id;
    }
}

static inline Entity* get_all_entities()
{
    return entities;
}

static inline uint16_t get_component_id(Entity entity, uint16_t type){
    for (TempID ids : entity.components) {
        if(ids.type == type) return ids.index;
    }
    return UINT16_MAX;
}

static inline void run_health_system(struct RenderPipeline* bad_design, HeapStack* heap_stack){
    ComponentSystem* health_system = get_component_system(HEALTH);
    ComponentSystem* transform_system = get_component_system(TRANSFORM);

    HealthComponent* health_comps = (HealthComponent*)get_component_by_id(health_system, 0);

    TransformComponent* transforms = (TransformComponent*)get_component_by_id(transform_system, 0);

    //Entity* entities = get_all_entities().data();

    for (int i = 0; i < health_system->amount; i++) {
        health_comps[i].health -= health_comps[i].damage_taken;
        health_comps[i].damage_taken = 0;
        if(health_comps[i].health <= 0){
            uint16_t transform_index = 0;

            if(has_component(entities[health_comps[i].entity_id], TRANSFORM, &transform_index)){


                transforms[transform_index].transform.position = {100000, 10000, 10};
                transforms[transform_index].transform.scale = {0.2, 0.2, 0.2};
                health_comps[i].health = 10;
            }
            remove_entity(health_comps[i].entity_id, bad_design, heap_stack);
        }
    }
}
