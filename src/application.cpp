#include "application.h"


VkDescriptorPool create_imgui_descriptor_pool(VkDevice virtual_device)
{
    VkDescriptorPool imgui_pool;

    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(virtual_device, &pool_info, nullptr, &imgui_pool);
    return imgui_pool;
}

void init_imgui(GLFWwindow* main_window, RenderPipeline* render_pipeline)
{
    VkDescriptorPool imgui_descriptor_pool = create_imgui_descriptor_pool(render_pipeline->get_device()->get_virtual_device());
    VkPhysicalDevice physical_device = render_pipeline->get_device()->get_physical_device();
    VkDevice virtual_device = render_pipeline->get_device()->get_virtual_device();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;
    
    ImGui_ImplGlfw_InitForVulkan(main_window, true);
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = render_pipeline->get_instance();
    init_info.PhysicalDevice = physical_device;
    init_info.Device = virtual_device;
    init_info.QueueFamily = Setup::find_queue_families(physical_device, render_pipeline->get_surface()).graphics_family.value();

    init_info.Queue = render_pipeline->get_device()->get_graphics_queue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imgui_descriptor_pool;
    init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = Setup::find_swap_chain_support(physical_device, render_pipeline->get_surface()).surface_capabilities.minImageCount + 1;
    init_info.Allocator = nullptr;
    init_info.PipelineInfoMain.RenderPass = render_pipeline->render_pass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.CheckVkResultFn = check_vk_result;

    ImGui_ImplVulkan_Init(&init_info);
}

Application::Application(const int width, const int height, const char* name) : application_name { name }
{
    render_pipeline = new RenderPipeline(width, height, application_name);

    main_window = render_pipeline->get_main_window();

    init_imgui(main_window, render_pipeline);
/**/
    
}

Application::~Application()
{
    cleanup();
}

void Application::main_game_loop()
{
    bool test = true;
    static auto start_time = std::chrono::high_resolution_clock::now();
    double frames = 0;


    Renderable render_this;
    render_this.mesh_location = "C:/Users/colin/Documents/Project/OstenEngine/GameEngine/assets/debug_assets/viking.obj";
    render_this.texture_location = "C:/Users/colin/Documents/Project/OstenEngine/GameEngine/assets/debug_assets/viking_room.png";

    //render_pipeline->draw_model(render_this);


    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();
        // render_pipeline->draw_frame();
        // continue;
        if (glfwGetWindowAttrib(main_window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }
        /**/
        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiWindowFlags window_flags = 0;
        ImGui::Begin("My Thing", &test, window_flags);
        ImGui::Text("My Thing! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
        ImGui::Spacing();
        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
        ImGui::Text("(%f)", ((float)frames / time));
        if(ImGui::Button("Swap Spin"))
        {
            render_pipeline->spin_direction = !render_pipeline->spin_direction;
        }
        ImGui::SliderFloat("XDir", &render_pipeline->spin_x, 0, 1);
        ImGui::SliderFloat("YDir", &render_pipeline->spin_y, 0, 1);
        ImGui::SliderFloat("ZDir", &render_pipeline->spin_z, 0, 1);

        ImGui::SliderFloat3("Camera_thing", render_pipeline->camera_thing, 0, 2000);

        ImGui::SliderFloat("Scale", &render_pipeline->scale, 0, 1);

        ImGui::End();

        // ImGui::ShowDemoWindow(&test);

        // ImDrawData* main_draw_data = ImGui::GetDrawData();
        
        render_pipeline->draw_frame();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        frames +=1;
    }
}

void Application::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    render_pipeline->cleanup();
}