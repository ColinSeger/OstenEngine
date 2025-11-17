#include "entity_manager.h"
namespace {
    std::vector<Entity> entities;    
}


void Entity_Manager::add_entity(Entity entity)
{
    entity.id = entities.size();
    entities.emplace_back(entity);
}

void Entity_Manager::remove_entity(Entity entity)
{
    //std::find(entities.begin(), entities.end(), entity);
}

void Entity_Manager::remove_entity(uint32_t entity)
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

uint32_t Entity_Manager::get_entity_amount()
{
    return entities.size();
}