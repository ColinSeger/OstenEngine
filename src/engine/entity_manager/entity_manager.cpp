// #include "entity_manager.h"
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
// #include "entity_system.cpp"

struct TempID
{
    uint16_t index = 0;
    uint16_t type = 0;
};

struct Entity{
    TempID components[5];
    uint16_t id;
    uint16_t amount;
};

void add_component(Entity& entity, TempID component){
    entity.components[entity.amount] = component;
    entity.amount++;
}

namespace EntityManager
{
    std::unordered_map<std::string, uint32_t>& get_entity_names();

    void add_entity(Entity entity, std::string name);

    // void add_component(uint32_t entity_id, uint8_t component, System& system);

    void remove_entity(Entity entity);

    void remove_entity(uint32_t entity);

    uint32_t get_entity_amount();

    void print_entities();

    void rename_entity(std::string current_name, std::string new_name);

    std::vector<Entity>& get_all_entities();
}

namespace {
    std::unordered_map<std::string, uint32_t> entity_names;
    std::vector<Entity> entities;
}
std::unordered_map<std::string, uint32_t>& EntityManager::get_entity_names(){
    return entity_names;
}

void EntityManager::add_entity(Entity entity, std::string name)
{
    auto contains = entity_names.find(name);
    while(contains != EntityManager::get_entity_names().end())
    {
        char buf[11];
        snprintf(buf, sizeof(buf), "%u", (uint32_t)entities.size());
        for(char c : buf){
            name.push_back(c);
        }
        for (uint8_t i = 1; i < 9; i++)
        {
            contains = entity_names.find(name);
            if(contains != EntityManager::get_entity_names().end()){
                uint32_t index = name.size()-1;
                name[index] = (char)i + '0';
            }
            else {
                break;
            }
        }
    }

    entity.id = entities.size();

    entities.emplace_back(entity);
    entity_names[name] = entity.id;
}

// void EntityManager::add_component(uint32_t entity_id, uint8_t component, System& system)
// {
//     TempID temp = TempID{
//         0,
//         (uint16_t)component
//     };
//     entities[entity_id].components.push_back(temp);
// }

void EntityManager::remove_entity(Entity entity)
{
    //std::find(entities.begin(), entities.end(), entity);
}

void EntityManager::remove_entity(uint32_t entity)
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

uint32_t EntityManager::get_entity_amount()
{
    return entities.size();
}

void rename_entity(std::string current_name, std::string new_name)
{
    auto contains = entity_names.find(current_name);
    if(contains != EntityManager::get_entity_names().end()){
        uint32_t id = entity_names[current_name];
        entity_names.erase(current_name);
        entity_names[new_name] = id;
    }
}

std::vector<Entity>& EntityManager::get_all_entities()
{
    return entities;
}

uint16_t get_component_id(Entity entity, uint16_t type){
    for (TempID ids : entity.components) {
        if(ids.type == type) return ids.index;
    }
    return UINT16_MAX;
}
