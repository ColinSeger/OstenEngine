#pragma once
#include <GLFW/glfw3.h>
#include "../../../external/math_3d.h"
#include "../../engine/entity_manager/components.h"


static inline void camera_movement(double delta_time, uint16_t camera_id, GLFWwindow* main_window){
    ComponentSystem* camera_system = get_component_system(CAMERA);
    ComponentSystem* transform_system = get_component_system(TRANSFORM);

    CameraComponent* camera = (CameraComponent*)get_component_by_id(camera_system, camera_id);
    TransformComponent* camera_tranform = (TransformComponent*)get_component_by_id(transform_system, camera->transform_id);

    int camera_speed = 20;

    if(glfwGetKey(main_window, GLFW_KEY_W) == GLFW_PRESS){
        camera_tranform->transform.position = v3_sub(camera_tranform->transform.position, v3_muls(v3_forward_vector(camera_tranform->transform), delta_time * camera_speed));
    }
    if(glfwGetKey(main_window, GLFW_KEY_S) == GLFW_PRESS){
        camera_tranform->transform.position = v3_add(camera_tranform->transform.position, v3_muls(v3_forward_vector(camera_tranform->transform), delta_time * camera_speed));
    }
    if(glfwGetKey(main_window, GLFW_KEY_A) == GLFW_PRESS){
        camera_tranform->transform.position = v3_add(camera_tranform->transform.position, v3_muls(v3_right_vector(camera_tranform->transform), delta_time * camera_speed));
    }
    if(glfwGetKey(main_window, GLFW_KEY_D) == GLFW_PRESS){
        camera_tranform->transform.position = v3_sub(camera_tranform->transform.position, v3_muls(v3_right_vector(camera_tranform->transform), delta_time * camera_speed));
    }
    if(glfwGetKey(main_window, GLFW_KEY_SPACE) == GLFW_PRESS){
        camera_tranform->transform.position.z += camera_speed * delta_time;
    }
    if(glfwGetKey(main_window, GLFW_KEY_C) == GLFW_PRESS){
        camera_tranform->transform.position.z -= camera_speed * delta_time;
    }

    if(glfwGetMouseButton(main_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        //camera_tranform->transform.position.x += 100 * delta_time;
    }
}

static float last_x = 0.0f;
static float last_y = 0.0f;
static float mouse_sensitivity = 0.01f;

static inline void camera_mouse_callback(GLFWwindow* main_window, double position_x, double position_y)
{
    if(glfwGetMouseButton(main_window, GLFW_MOUSE_BUTTON_MIDDLE) != GLFW_PRESS){
        last_x = (float)position_x;
        last_y = (float)position_y;
        return;
    }
    float x_offset = last_x - (float)position_x;
    float y_offset = (float)position_y - last_y;

    last_x = (float)position_x;
    last_y = (float)position_y;

    ComponentSystem* camera_sys = get_component_system(CAMERA);
    ComponentSystem* transform_sys = get_component_system(TRANSFORM);
    CameraComponent* camera = (CameraComponent*)get_component_by_id(camera_sys, 0);

    TransformComponent* camera_transform = (TransformComponent*)get_component_by_id(transform_sys, camera->transform_id);

    x_offset *= mouse_sensitivity;
    y_offset *= mouse_sensitivity;

    camera_transform->transform.rotation.y += x_offset;
    camera_transform->transform.rotation.x += y_offset;

    // Clamp pitch to avoid flipping
    float limit = 1.55f; // ~89 degrees//AI said so at least TODO Check for real
    if(camera_transform->transform.rotation.x > limit)
        camera_transform->transform.rotation.x = limit;
    if(camera_transform->transform.rotation.x < -limit)
        camera_transform->transform.rotation.x = -limit;
}
