#pragma once
#include "../render_data/vulkan/render_data.h"

struct Camera
{
    Vector3 position;
    Vector3 look_location;
};


namespace Cameras{

    void camera_movement();

}
