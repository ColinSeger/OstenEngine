// #include "entity_manager.h"
#pragma once
#include "components.h"
#include <string.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_map>
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
    std::vector<Entity> entities;
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
        snprintf(buf, sizeof(buf), "%u", (uint32_t)entities.size());
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

    entity.id = entities.size();

    for(TempID component : entity.components){
        ComponentSystem* system = get_component_system(component.type);
        Component* comp = (Component*)get_component_by_id(system, component.index);
        comp->entity_id = entity.id;
    }

    entities.emplace_back(entity);
    entity_names[name] = entity.id;
}

static inline void remove_entity(Entity entity)
{
    //std::find(entities.begin(), entities.end(), entity);
}

static inline void remove_entity(uint32_t entity)
{
    for (size_t i = 0; i < entities.size(); i++)
    {
        if(entities[i].id == entity)
        {
            entities.erase(entities.begin() + i);
            return;
        }
    }
}

static inline uint32_t get_entity_amount()
{
    return entities.size();
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

static inline std::vector<Entity>& get_all_entities()
{
    return entities;
}

static inline uint16_t get_component_id(Entity entity, uint16_t type){
    for (TempID ids : entity.components) {
        if(ids.type == type) return ids.index;
    }
    return UINT16_MAX;
}

static inline void run_health_system(){
    ComponentSystem* health_system = get_component_system(HEALTH);
    ComponentSystem* transform_system = get_component_system(TRANSFORM);

    HealthComponent* health_comps = (HealthComponent*)get_component_by_id(health_system, 0);

    TransformComponent* transforms = (TransformComponent*)get_component_by_id(transform_system, 0);

    Entity* entities = get_all_entities().data();

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
        }
    }
}
