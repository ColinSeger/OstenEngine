// #include "entity_system.h"
#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "components.cpp"

struct System
{
    uint32_t amount = 0;
    uint32_t capacity = 0;
    uint16_t type = 0;
    Component* components = 0;
    static uint8_t stub_system(System& system){ return 0;};
    uint8_t (*run_system)(System&) = &stub_system;
};

uint8_t init_system(System& system, uint16_t component_size, uint32_t capacity)
{
    system.amount = 0;
    system.capacity = capacity;
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

    uint16_t size_of_component = get_component_size_by_type(system.type);

    system.components = (Component*)malloc(capacity * size_of_component);

    return 1;
}

uint8_t add_action(System& system, uint8_t (*function)(System&))
{
    if(function == nullptr) return 0;
    system.run_system = function;
    return 1;
}

uint8_t add_component(System& system, void* component)
{
    uint16_t type = static_cast<Component*>(component)->id;
    if(system.type == type && system.amount < system.capacity){
        char* comp = (char*)system.components;
        uint16_t size = get_component_size_by_type(type);
        uint32_t size_offset = size * system.amount;
        comp += size_offset;
        memcpy(comp , component, size);
        system.amount++;
        return 1;
    }

    return 0;
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
