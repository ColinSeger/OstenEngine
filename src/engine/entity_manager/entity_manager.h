#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include "components.h"
#include "entity_system.h"


static void stub(){}

struct TempID
{
    uint32_t index = 0;
    uint16_t type = 0;
};

struct Entity{
    uint32_t id;
    std::vector<TempID> components;
    void (*test)() = stub;
};

namespace EntityManager
{
    std::unordered_map<std::string, uint32_t>& get_entity_names();

    void add_entity(Entity entity, std::string name);

    void add_component(uint32_t entity_id, Type component, System& system);

    void remove_entity(Entity entity);

    void remove_entity(uint32_t entity);

    uint32_t get_entity_amount();

    void print_entities();

    void rename_entity(std::string current_name, std::string new_name);

    std::vector<Entity>& get_all_entities();
}
