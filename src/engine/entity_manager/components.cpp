// #include "components.h"
#pragma once
#include <cstdint>
#include <cstdlib>
#include "../transform.cpp"

constexpr uint8_t CAMERA = 0;
constexpr uint8_t TRANSFORM = 1;
constexpr uint8_t RENDER = 2;


typedef struct
{
    const uint16_t id;
} Component;

struct TransformComponent
{
    const uint16_t id = 1;
    Transform transform {};
    TransformComponent operator=(TransformComponent transform){
        this->transform = transform.transform;
        return *this;
    }
};

struct RenderComponent
{
    const uint16_t id = 2;
    uint16_t transform_id = 0;
    uint16_t descriptor_id = 0;
    uint16_t mesh_id = 0;
    uint16_t texture_id = 0;
};

struct CameraComponent
{
    const uint16_t id = 3;
    uint16_t transform_id = 0;
    float field_of_view = 45.f;

    CameraComponent operator=(CameraComponent camera){
        this->transform_id = camera.transform_id;
        this->field_of_view = camera.field_of_view;
        return *this;
    }
};

struct ComponentSystem
{
    void* components;
    uint16_t amount = 0;
    uint16_t capacity = 10;
    uint8_t type = 0;
};

namespace{
    ComponentSystem cameras{};
    ComponentSystem transforms{};
    ComponentSystem render_components{};
}

uint16_t get_component_size_by_type(uint16_t type){
    switch ((uint8_t)type)
    {
    case TRANSFORM:
        return sizeof(TransformComponent);

    case RENDER:
        return sizeof(RenderComponent);

    case CAMERA:
        return sizeof(CameraComponent);

    default:
        return 0;
        break;
    }
}

void* get_component_by_id(ComponentSystem* component_system, uint16_t id)
{
    char* comp = (char*)component_system->components;
    uint16_t size = get_component_size_by_type(component_system->type);
    uint32_t size_offset = size * id;
    comp += size_offset;
    return comp;
}

ComponentSystem* get_component_system(uint8_t system_id)
{
    switch (system_id)
    {
    case CAMERA:
        return &cameras;
        break;
    case TRANSFORM:
        return &transforms;
        break;
    case RENDER:
        return &render_components;
        break;
    default:
        return nullptr;
        break;
    }
}
// ComponentSystem* get_component_system(void* system_component)
// {
//     uint16_t type = reinterpret_cast<Component*>(system_component)->id;
//     switch (type)
//     {
//     case 0:
//         return &cameras;
//         break;
//     case 1:
//         return &transforms;
//         break;

//     default:
//         return nullptr;
//         break;
//     }
// }

void create_transform_system(uint8_t transform_amount){
    ComponentSystem* component_sys = get_component_system(TRANSFORM);
    component_sys->components = malloc(sizeof(TransformComponent) * transform_amount);
    component_sys->type = 1;
    for (size_t i = 0; i < transform_amount; i++)
    {
        TransformComponent* comp = (TransformComponent*)get_component_by_id(component_sys, i);
        comp->transform = Transform{};
    }
     //Component* test = reinterpret_cast<Component*>(component_sys->components);
}

uint16_t add_transform()
{
    ComponentSystem* component_sys = get_component_system(TRANSFORM);
    char* comp = (char*)component_sys->components;
    uint16_t size = get_component_size_by_type(TRANSFORM);
    uint32_t size_offset = size * component_sys->amount;
    comp += size_offset;
    component_sys->amount++;
    return component_sys->amount-1;
}

void create_render_component_system(uint8_t render_amount){
    ComponentSystem* component_sys = get_component_system(RENDER);
    component_sys->components = malloc(sizeof(RenderComponent) * render_amount);
    component_sys->type = 2;
    component_sys->capacity = render_amount;
    for (size_t i = 0; i < render_amount; i++)
    {
        RenderComponent* comp = (RenderComponent*)get_component_by_id(component_sys, i);
        comp->transform_id = add_transform();
        comp->mesh_id = 0;
        comp->texture_id = 0;
    }
     //Component* test = reinterpret_cast<Component*>(component_sys->components);
}

uint16_t add_render_component(uint16_t descriptor_index)
{
    ComponentSystem* component_sys = get_component_system(RENDER);
    RenderComponent* comp = reinterpret_cast<RenderComponent*>(component_sys->components);
    // uint16_t size = get_component_size_by_type(RENDER);
    uint32_t size_offset = component_sys->amount;
    comp += size_offset;
    component_sys->amount++;
    comp->descriptor_id = descriptor_index;
    return component_sys->amount-1;
}

void create_camera_system(uint8_t camera_amount){
    ComponentSystem* component_sys = get_component_system(CAMERA);
    component_sys->components = malloc(sizeof(CameraComponent) * camera_amount);
    component_sys->amount = camera_amount;
    component_sys->type = 0;

    for (size_t i = 0; i < camera_amount; i++)
    {
        CameraComponent* comp = (CameraComponent*)get_component_by_id(component_sys, i);
        *comp = CameraComponent{};
        comp->transform_id = add_transform();
        TransformComponent* transform = (TransformComponent*)get_component_by_id(&transforms, comp->transform_id);
        transform->transform.position.x = 10;
        transform->transform.position.z = 2;

        transform->transform.rotation.y = 1.3f;
    }
}

void loop_over_component(ComponentSystem* system)
{

}
