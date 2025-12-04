#pragma once
#include "../../../external/glm/glm.hpp"
#include "../../../external/glm/gtc/matrix_transform.hpp"
#include "../render_data/vulkan/render_data.h"
// #include "../../engine/transform.h"

struct Camera
{
    Vector3 position;
    Vector3 look_location;
};


namespace Cameras{

    void camera_movement();

}