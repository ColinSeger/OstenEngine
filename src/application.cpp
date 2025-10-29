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
    

    instance = new Instance(application_name, false);
    // instance = &new_instance;
    create_surface();
    device = new Device(instance->get_instance(), surface);
    // device = &dev;

}

Application::~Application()
{
    cleanup();
    delete instance;
    delete device;
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

    // vkDestroySurfaceKHR(instance->get_instance(), surface, nullptr);
    vkDestroyInstance(instance->get_instance(), nullptr);

    glfwTerminate();
}


void Application::create_surface(){
    assert(glfwCreateWindowSurface(instance->get_instance(), main_window, nullptr, &surface) == VK_SUCCESS);
}