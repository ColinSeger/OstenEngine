#pragma once
#include <iostream>
#include <vector>
#include "../transform.h"
#include "../../renderer/renderable.h"


static void stub(){}

struct Entity{
    uint32_t id;
    Transform transform;
    Renderable rendering;
    void (*test)() = stub;
};

namespace Entity_Manager
{
    void add_entity(Entity entity);

    void remove_entity(Entity entity);

    void remove_entity(uint32_t entity);

    uint32_t get_entity_amount();

    void print_entities();

    std::vector<Entity> get_all_entities();
}
