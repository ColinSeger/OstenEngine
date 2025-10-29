#include "application.h"


Application::Application(const int width, const int height, const char* name) : application_name { name }
{
    assert(glfwInit() == true && "GLFW Failed to open");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);


    #ifdef NDEBUG
        const bool enable_validation = false;
    #else
        const bool enable_validation = true;
    #endif
    

    Instance test = Instance(application_name, false);
    instance = &test;
}

Application::~Application()
{
    cleanup();
}

void Application::main_game_loop()
{
    
    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();
    }

    cleanup();
}

void Application::cleanup()
{
    if(!main_window) return;
    glfwDestroyWindow(main_window);

    glfwTerminate();    
}