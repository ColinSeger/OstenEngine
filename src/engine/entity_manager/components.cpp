// #include "components.h"
#pragma once
#include <cstdint>
#include "../../../external/imgui_test/imgui.h"
#include "../transform.cpp"

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

namespace{
    ComponentSystem cameras{};
    ComponentSystem transforms{};
}

uint16_t get_component_size_by_type(uint16_t type){
    switch ((Type)type)
    {
    case Type::Component:
        return sizeof(Component);

    case Type::Transform:
        return sizeof(TransformComponent);

    case Type::Render:
        return sizeof(RenderComponent);

    case Type::Camera:
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
    case 0:
        return &cameras;
        break;
    case 1:
        return &transforms;
        break;

    default:
        return nullptr;
        break;
    }
}

void create_camera_system(uint8_t camera_amount){
    ComponentSystem* component_sys = get_component_system(0);
    component_sys->components = (Component*)malloc(sizeof(CameraComponent) * camera_amount);
    component_sys->amount = camera_amount;
    component_sys->type = 0;
    for (size_t i = 0; i < camera_amount; i++)
    {
        CameraComponent* comp = (CameraComponent*)get_component_by_id(component_sys, i);
        *comp = CameraComponent{};
    }
}

void create_transform_system(uint8_t transform_amount){
    TransformComponent test {};
    ComponentSystem* component_sys = get_component_system(1);
    component_sys->components = (Component*)malloc(sizeof(TransformComponent) * transform_amount);
    component_sys->type = 1;
    for (size_t i = 0; i < transform_amount; i++)
    {
        TransformComponent* comp = (TransformComponent*)get_component_by_id(component_sys, i);
        comp->transform = Transform{};
    }
}

uint16_t add_transform()
{
    ComponentSystem* component_sys = get_component_system(1);
    char* comp = (char*)component_sys->components;
    uint16_t size = get_component_size_by_type((uint16_t)Type::Transform);
    uint32_t size_offset = size * component_sys->amount;
    comp += size_offset;
    component_sys->amount++;
    return component_sys->amount-1;
}

void inspect(uint8_t type, uint16_t id)
{
    switch (type)
    {
    case 0:
        ImGui::Text("Camera");
        ImGui::DragFloat3("Camera Position", &static_cast<CameraComponent*>(get_component_by_id(&cameras, id))->transform.position.x, 0.1f);
        ImGui::DragFloat3("Camera Rotation", &static_cast<CameraComponent*>(get_component_by_id(&cameras, id))->transform.rotation.x, 0.1f);
        ImGui::DragFloat("Fov", &static_cast<CameraComponent*>(get_component_by_id(&cameras, id))->fov, 0.1f);
        break;
    case 1:
        ImGui::Text("Transform");
        ImGui::DragFloat3("Position", &static_cast<TransformComponent*>(get_component_by_id(&transforms, id))->transform.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &static_cast<TransformComponent*>(get_component_by_id(&transforms, id))->transform.rotation.x, 0.1f);
        ImGui::DragFloat3("Scale", &static_cast<TransformComponent*>(get_component_by_id(&transforms, id))->transform.scale.x, 0.1f);
        break;

    default:
        break;
    }
}

void loop_over_component(ComponentSystem* system)
{

}
