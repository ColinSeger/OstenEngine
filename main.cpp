// #include "external/vulkan/vulkan.h"
#include <iostream>
// #include <glm/glm.hpp>
// #define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// #include "external/glfw-3.4/include/GLFW/glfw3.h"
#include <cassert>

int main()
{
    
    assert(glfwInit() == true && "GLFW Failed to open");

    /*
    if(!glfwInit()){
        return 1;
    }
    */
    /**/
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    
    std::string result;
    std::cout << "Hello World \n";
    std::cin >> result;
    // glm::vec2 vector2;
    
    return 0;
}