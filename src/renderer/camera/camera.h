#pragma once
#include "../../../external/glm/glm.hpp"
#include "../../../external/glm/gtc/matrix_transform.hpp"

struct Camera
{
    glm::vec3 position;
    glm::vec3 look_location;
};


namespace camera{

    void camera_movement();


}