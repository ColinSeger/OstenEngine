#include "camera.h"

void Cameras::camera_movement()
{

}

Vector3 Cameras::forward_vector(Transform transform)
{
    return {
        cos(transform.rotation.z) * sin(transform.rotation.y),
        -sin(transform.rotation.z),
        cos(transform.rotation.z) * cos(transform.rotation.y)
    };
}

Vector3 Cameras::right_vector(Transform transform)
{
    return
    {
        cos(transform.rotation.y),
        0,
        -sin(transform.rotation.y)
    };
}

Vector3 Cameras::up_vector(Transform transform)
{
    Vector3 f = forward_vector(transform);
    Vector3 r = right_vector(transform);
    glm::vec3 forward = {f.x, f.y, f.z};
    glm::vec3 right = {r.x, r.y, r.z};
    glm::vec3 res = glm::cross(forward, right);
    // return cross(forward_vector(transform), right_vector(transform));

    return Vector3{res.x, res.y, res.z};
}