#pragma once
#include "../transform.h"


enum class Type{
    Component = 0,
    Transform = 1,
    Render = 2
};


struct Component
{
    const uint16_t id = 0; 
};

struct TransformComponent
{
    const uint16_t id = 1;
    Transform transform {};
};

struct RenderComponent
{
    const uint16_t id = 2;
};

uint16_t get_component_size_by_type(uint16_t type);
