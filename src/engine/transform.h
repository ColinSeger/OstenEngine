#pragma once
#include "../../external/glm/glm.hpp"
#include "../../external/glm/gtc/matrix_transform.hpp"

struct Transform
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};

namespace Transformations
{
    glm::mat4 get_model_matrix(Transform transform);
}

//uint8_t print_transform(System& system);