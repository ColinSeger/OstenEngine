#include "components.h"


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