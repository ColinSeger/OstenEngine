#pragma once
#include <GLFW/glfw3.h>
#include "../../../external/math_3d.h"
#include "../../engine/entity_manager/components.cpp"

static double xpos, ypos;

inline void camera_movement(double delta_time, uint16_t camera_id, GLFWwindow* window){
    ComponentSystem* camera_system = get_component_system(CAMERA);
    ComponentSystem* transform_system = get_component_system(TRANSFORM);

    CameraComponent* camera = (CameraComponent*)get_component_by_id(camera_system, camera_id);
    TransformComponent* camera_tranform = (TransformComponent*)get_component_by_id(transform_system, camera->transform_id);

    int camera_speed = 20;

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camera_tranform->transform.position.x -= camera_speed * delta_time;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        camera_tranform->transform.position.x += camera_speed * delta_time;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        camera_tranform->transform.position.y -= camera_speed * delta_time;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        camera_tranform->transform.position.y += camera_speed * delta_time;
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        //camera_tranform->transform.position.x += 100 * delta_time;
    }
    static double old_xpos = xpos, old_ypos = ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if(old_xpos < ypos){
        //camera_tranform->transform.rotation.y += 1 * delta_time;
    }
}
