#include "render_pipeline.h"


RenderPipeline::RenderPipeline(const int width, const int height, const char* application_name)
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

    assert(glfwCreateWindowSurface(instance->get_instance(), main_window, nullptr, &surface) == VK_SUCCESS);

    device = new Device(instance->get_instance(), surface, enable_validation);
    swap_chain = new SwapChain(main_window, device->get_physical_device(), surface, device->get_virtual_device());
}

RenderPipeline::~RenderPipeline()
{
    cleanup();
}

void RenderPipeline::cleanup()
{
    glfwDestroyWindow(main_window);
    delete swap_chain;
    vkDestroySurfaceKHR(instance->get_instance(), surface, nullptr);
    delete instance;
    delete device;
    glfwTerminate();
}