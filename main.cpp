#pragma once
#include <iostream>
// #include <glm/glm.hpp>
#include "external/glm/glm.hpp"
#include "src/application.cpp"

int main()
{   
    glm::vec2 vector2 = {1, 0};
    const char* name = "Vulkan Window";

    Application app = Application(800, 600, name);

    app.main_game_loop();

    std::string result;
    std::cout << "Hello World \n" << vector2.x;
    std::cin >> result;
    
    return 0;
}