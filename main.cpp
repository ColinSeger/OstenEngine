#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "external/glm/glm.hpp"
#include "src/application.h"

int main()
{   
    glm::vec2 vector2 = {1, 0};
    const char* name = "Vulkan Window";

    Application app = Application(800, 600, name);

    app.main_game_loop();

    // std::string result;
    std::cout << "Hello World \n" << vector2.x;
    // std::cin >> result;
    
    return 0;
}