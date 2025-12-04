#include "entity_system.h"


uint8_t init_system(System& system, uint16_t component_size, uint32_t capacity)
{
    system.amount = 0;
    system.capacity = capacity;
    system.size_of_component = component_size;
    system.components = (Component*)malloc(capacity * component_size);

    return 1;
}

uint8_t init_system(System& system, void* component, uint32_t capacity)
{
    Component* cmp = (Component*)component;
    if(cmp->id <= 0){
        return 0;
    }
    system.amount = 0;
    system.capacity = capacity;
    system.type = cmp->id;
    
    system.size_of_component = get_component_size_by_type(system.type);

    system.components = (Component*)malloc(capacity * system.size_of_component);

    return 1;
}

uint8_t add_action(System& system, uint8_t (*function)(System&))
{
    if(function == nullptr) return 0;
    system.run_system = function;
    return 1;
}

uint16_t add_component(System& system, void* component)
{
    uint16_t type = static_cast<Component*>(component)->id;
    if(system.type == type && system.amount < system.capacity){
        char* comp = (char*)system.components;
        uint16_t size = get_component_size_by_type(type);
        uint32_t size_offset = size * system.amount;
        comp += size_offset;
        memcpy(comp , component, size);
        system.amount++;    
    }

    return 1;
}


uint8_t debug(System& system){
    TransformComponent* t = (TransformComponent*)system.components;
    for (size_t i = 0; i < system.amount; i++)
    {
        printf("Test %f \n", t->transform.position.x);
        t->transform.position.x += 0.001f;
        t++;
    }
    return 1;
}

uint8_t destroy_system(System& system)
{
    system.amount = 0;
    system.run_system = system.stub_system;
    system.type = 0;
    
    free(system.components);

    return 1;
}