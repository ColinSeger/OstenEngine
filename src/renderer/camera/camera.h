#pragma once
#include "../../../external/math_3d.h"
#include "../../engine/entity_manager/components.cpp"

inline void camera_movement(double delta_time, uint16_t camera_id){
    ComponentSystem* camera_system = get_component_system(CAMERA);
    ComponentSystem* transform_system = get_component_system(TRANSFORM);

    CameraComponent* camera = (CameraComponent*)get_component_by_id(camera_system, camera_id);
    TransformComponent* camera_tranform = (TransformComponent*)get_component_by_id(transform_system, camera->transform_id);


}
