#pragma once
#include <cstdint>
#include <cstdlib>



struct Component
{
};


struct System
{
    uint32_t amount = 0;
    uint16_t size_of_component = 0;
    Component* components = 0;
    static uint8_t stub_system(System& system){ return 0;};
    uint8_t (*run_system)(System&) = &stub_system;
};

uint8_t add_action(System& system, uint8_t (*function)(System&));

uint8_t init_system(System& system, uint16_t component_size, uint32_t capacity); 
// int add_component(Component component);