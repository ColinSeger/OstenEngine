#include "application.h"


Application::Application(const int width, const int height, const char* name) : application_name { name }
{
    render_pipeline = new RenderPipeline(width, height, application_name);

    main_window = render_pipeline->get_main_window();
/*
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(width);
    style.FontScaleDpi = width;
    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;*/
}

Application::~Application()
{
    cleanup();
}

void Application::main_game_loop()
{
    bool test = true;
    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();
        // ImGui::ShowDemoWindow(&test);
        render_pipeline->draw_frame();
    }
}

void Application::cleanup()
{
    render_pipeline->cleanup();
}