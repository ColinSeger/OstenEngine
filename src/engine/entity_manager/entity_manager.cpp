#include "entity_manager.h"
namespace {
    std::unordered_map<std::string, uint32_t> entity_names;
    std::vector<Entity> entities;    
}
std::unordered_map<std::string, uint32_t> Entity_Manager::get_entity_names(){
    return entity_names;
}
// Entity_Manager::std::unordered_map<std::string, uint32_t> entity_names;
void hello(){
    std::cout << "Hello\n";
}

void test(){
    std::cout << "Test\n";
}

void Entity_Manager::add_entity(Entity entity, std::string name)
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

void Entity_Manager::add_component(uint32_t entity_id, Type component, System& system)
{
    TempID temp = TempID{
        0,
        (uint16_t)component
    };
    entities[entity_id].components.push_back(temp);
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

std::vector<Entity>& Entity_Manager::get_all_entities()
{
    return entities;
}