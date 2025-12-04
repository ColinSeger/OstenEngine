#include "entity_manager.h"
namespace {
    std::unordered_map<std::string, uint32_t> entity_names;
    std::vector<Entity> entities;    
}
std::unordered_map<std::string, uint32_t> EntityManager::get_entity_names(){
    return entity_names;
}
void hello(){
    std::cout << "Hello\n";
}

void test(){
    std::cout << "Test\n";
}

void EntityManager::add_entity(Entity entity, std::string name)
{
    entity.id = entities.size();
    
    if(entities.size() > 2){
        entity.test = &hello;
    }else{
        entity.test = &test;
    }
    entities.emplace_back(entity);
    entity_names[name] = entity.id;
}

void EntityManager::add_component(uint32_t entity_id, Type component, System& system)
{
    TempID temp = TempID{
        0,
        (uint16_t)component
    };
    entities[entity_id].components.push_back(temp);
}

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

void EntityManager::print_entities()
{
    for (size_t i = 0; i < entities.size(); i++)
    {
        entities[i].test();
    }
    
}

std::vector<Entity>& EntityManager::get_all_entities()
{
    return entities;
}