#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define TINYOBJLOADER_IMPLEMENTATION
// #include "external/glm/glm.hpp"
#include "external/imgui/imgui.h"
#include "src/application.h"

int main()
{   
    const char* name = "Vulkan Window";

    Application app = Application(800, 600, name);

    app.main_game_loop();

    // std::string result;
    std::cout << "Hello World \n";
    // std::cin >> result;
    
    return 0;
}