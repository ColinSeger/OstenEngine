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

uint16_t add_component(System& system, void* component)
{
    uint16_t type = static_cast<Component*>(component)->id;
    if(system.type == type){
        char* comp = (char*)system.components;
        uint16_t size = get_component_size_by_type(type);
        uint32_t size_offset = size * system.amount++;
        comp += size_offset;
        memcpy(comp , component, size);        
    }

    return 0;
}


uint8_t debug(System& system){
    TransformComponent* t = (TransformComponent*)system.components;
    for (size_t i = 0; i < system.amount; i++)
    {
        printf("Test %f \n", t->transform.position.x);
        t++;
    }
    return 1;
}