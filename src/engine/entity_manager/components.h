#pragma once
#include "../transform.h"
#include "../../renderer/camera/camera.h"


enum class Type : uint16_t{
    Component = 0,
    Transform = 1,
    Render = 2,
    Camera = 3
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

struct CameraComponent
{
    const uint16_t id = 3;
    Camera camera;
};

uint16_t get_component_size_by_type(uint16_t type);
