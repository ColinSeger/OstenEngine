#include <iostream>
// #include <glm/glm.hpp>
#include "external/glm/glm.hpp"
#include "src/application.cpp"

int main()
{   
    glm::vec2 vector2 = {1, 0};

    application::run();

    std::string result;
    std::cout << "Hello World \n" << vector2.x;
    std::cin >> result;
    
    return 0;
}