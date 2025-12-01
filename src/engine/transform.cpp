#include "transform.h"

glm::mat4 Transformations::get_model_matrix(Transform transform)//This should be a for loop inside renderer probably
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, transform.position);
    model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, transform.scale);
    return model;
}

uint8_t print_transform(System& system)
{
    Transform* trans = (Transform*)system.components;
    for (size_t i = 0; i < 3; i ++)
    {
        printf("Printing Transform %f \n", trans->position.x);
        trans += sizeof(Transform);
    }
    return 0;
}