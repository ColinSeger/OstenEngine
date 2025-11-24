#include "entity_manager.h"
namespace {
    std::vector<Entity> entities;    
}

void hello(){
    std::cout << "Hello\n";
}

void test(){
    std::cout << "Test\n";
}

void Entity_Manager::add_entity(Entity entity)
{
    entity.id = entities.size();
    
    if(entities.size() > 2){
        entity.test = &hello;
    }else{
        entity.test = &test;
    }
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

void Entity_Manager::print_entities()
{
    for (size_t i = 0; i < entities.size(); i++)
    {
        entities[i].test();
    }
    
}