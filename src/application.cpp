#include "application.h"


Application::Application(const int width, const int height, const char* name) : application_name { name }
{
    render_pipeline = new RenderPipeline(width, height, application_name);

    main_window = render_pipeline->get_main_window();
}

Application::~Application()
{
    cleanup();
}

void Application::main_game_loop()
{
    
    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();
        render_pipeline->draw_frame();
    }

    cleanup();
}

void Application::cleanup()
{
    render_pipeline->cleanup();
}