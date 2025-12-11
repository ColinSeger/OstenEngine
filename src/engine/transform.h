#pragma once
#include "../../external/math_3d.h"
#include "../renderer/render_data/vulkan/render_data.h"

struct Transform
{
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 rotation = {0.0f, 0.0f, 0.0f};
    Vector3 scale = {1.0f, 1.0f, 1.0f};
};

namespace Transformations
{
    mat4_t  get_model_matrix(Transform transform);

    Vector3 forward_vector(Transform transform);

    Vector3 right_vector(Transform transform);

    Vector3 up_vector(Transform transform);
}

//uint8_t print_transform(System& system);
