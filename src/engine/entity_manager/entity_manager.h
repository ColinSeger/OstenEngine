#pragma once
#include <vector>
#include "../transform.h"
#include "../../renderer/renderable.h"


struct Entity{
    uint32_t id;
    Transform transform;
    Renderable rendering;
};

namespace Entity_Manager
{
    void add_entity(Entity entity);

    void remove_entity(Entity entity);

    void remove_entity(uint32_t entity);

    uint32_t get_entity_amount();
}
