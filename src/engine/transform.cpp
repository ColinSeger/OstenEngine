#include "transform.h"

glm::mat4 Transformations::get_model_matrix(Transform transform)//This should be a for loop inside renderer probably
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, {transform.position.x, transform.position.y, transform.position.z});
    model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, transform.scale);
    return model;
}


Vector3 Transformations::forward_vector(Transform transform)
{
    return {
        cos(transform.rotation.z) * sin(transform.rotation.y),
        -sin(transform.rotation.z),
        cos(transform.rotation.z) * cos(transform.rotation.y)
    };
}

Vector3 Transformations::right_vector(Transform transform)
{
    return
    {
        cos(transform.rotation.y),
        0,
        -sin(transform.rotation.y)
    };
}

Vector3 Transformations::up_vector(Transform transform)
{
    Vector3 f = forward_vector(transform);
    Vector3 r = right_vector(transform);
    glm::vec3 forward = {f.x, f.y, f.z};
    glm::vec3 right = {r.x, r.y, r.z};
    glm::vec3 res = glm::cross(forward, right);
    // return cross(forward_vector(transform), right_vector(transform));

    return Vector3{res.x, res.y, res.z};
}
