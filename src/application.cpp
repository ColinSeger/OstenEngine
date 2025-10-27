#include "application.h"


Application::Application(const int width, const int height, const char* name)
{
    assert(glfwInit() == true && "GLFW Failed to open");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, name, nullptr, nullptr);
}

Application::~Application()
{
    glfwDestroyWindow(main_window);

    glfwTerminate();
}

void Application::main_game_loop()
{
    
    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();
    }
}
