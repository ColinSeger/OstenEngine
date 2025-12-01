#include "entity_system.h"


uint8_t init_system(System& system, uint16_t component_size, uint32_t capacity)
{
    system.amount = 0;
    system.size_of_component = component_size;
    system.components = (Component*)malloc(capacity * component_size);

    return 1;
}

uint8_t add_action(System& system, uint8_t (*function)(System&))
{
    if(function == nullptr) return 0;
    system.run_system = function;
    return 1;
}

