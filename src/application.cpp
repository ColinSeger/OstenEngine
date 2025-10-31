#include "application.h"


Application::Application(const int width, const int height, const char* name) : application_name { name }
{
    assert(glfwInit() == GLFW_TRUE && "GLFW Failed to open");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);


    #ifdef NDEBUG
        const bool enable_validation = false;
    #else
        const bool enable_validation = true;
    #endif
    

    instance = new Instance(application_name, enable_validation);
    // instance = &new_instance;
    create_surface();
    device = new Device(instance->get_instance(), surface, enable_validation);
    // device = &dev;
    swap_chain = new SwapChain(main_window, device->get_physical_device(), surface, device->get_virtual_device());
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
    // if(!main_window) return;
    glfwDestroyWindow(main_window);
    delete swap_chain;
    vkDestroySurfaceKHR(instance->get_instance(), surface, nullptr);
    delete instance;
    delete device;
    glfwTerminate();
}


void Application::create_surface(){
    assert(glfwCreateWindowSurface(instance->get_instance(), main_window, nullptr, &surface) == VK_SUCCESS);
}