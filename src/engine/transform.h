#pragma once
//#include "../../external/math_3d.h"
#include "../../external/glm/glm.hpp"
#include "../../external/glm/gtc/matrix_transform.hpp"
#include "../renderer/render_data/vulkan/render_data.h"

struct Transform
{
    Vector3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};

namespace Transformations
{
    glm::mat4 get_model_matrix(Transform transform);

    Vector3 forward_vector(Transform transform);

    Vector3 right_vector(Transform transform);

    Vector3 up_vector(Transform transform);
}

//uint8_t print_transform(System& system);
