#pragma once
#include "../../common_includes.h"
#include "../transform.h"


enum class Type : uint8_t{
    Component = 0,
    Transform = 1,
    Render = 2,
    Camera = 3
};


typedef struct Component
{
    const uint16_t id = 0;
} Component;

typedef struct TransformComponent
{
    const uint16_t id = 1;
    Transform transform {};
    TransformComponent operator=(TransformComponent transform){
        this->transform = transform.transform;
        return *this;
    }
} TransComponent;

struct RenderComponent
{
    const uint16_t id = 2;
};

typedef struct CameraComponent
{
    const uint16_t id = 3;
    Transform transform;
    float fov = 45.f;

    CameraComponent operator=(CameraComponent camera){
        this->transform = camera.transform;
        this->fov = camera.fov;
        return *this;
    }
} CamComponent;

struct ComponentSystem
{
    Component* components;
    uint16_t amount = 0;
    uint16_t capacity = 10;
    uint8_t type = 0;
};

uint16_t get_component_size_by_type(uint16_t type);

ComponentSystem* get_component_system(uint8_t system_id);

void create_camera_system(uint8_t camera_amount);

void create_transform_system(uint8_t transform_amount);

uint16_t add_transform();

void inspect(uint8_t type, uint16_t id);

void* get_component_by_id(ComponentSystem* component_system, uint16_t id);
