#include "application.h"


void init_imgui(GLFWwindow* main_window, RenderPipeline* render_pipeline)
{
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
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;

    ImGui_ImplVulkanH_Window g_MainWindowData;
    VkPhysicalDevice physical_device = render_pipeline->get_device()->get_physical_device();
    VkDevice virtual_device = render_pipeline->get_device()->get_virtual_device();
    VkQueue g_Queue = VK_NULL_HANDLE;
    uint32_t g_QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(physical_device);
    int w, h;
    glfwGetFramebufferSize(main_window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    wd->Surface = render_pipeline->get_surface();
    // SetupVulkanWindow(wd, render_pipeline->get_surface(), w, h);
     // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physical_device, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physical_device, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    int width, height;
    glfwGetFramebufferSize(main_window, &width, &height);

    ImGui_ImplVulkanH_CreateOrResizeWindow(render_pipeline->get_instance(), physical_device, virtual_device, wd, g_QueueFamily, nullptr, width, height, MAX_FRAMES_IN_FLIGHT, 0);

    

    ImGui_ImplGlfw_InitForVulkan(main_window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = render_pipeline->get_instance();
    init_info.PhysicalDevice = physical_device;
    init_info.Device = virtual_device;
    
    init_info.QueueFamily = g_QueueFamily;

    vkGetDeviceQueue(virtual_device, g_QueueFamily, 0, &g_Queue);
    init_info.Queue = g_Queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = render_pipeline->get_descriptor_pool();
    init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = wd->ImageCount;
    init_info.Allocator = nullptr;
    init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
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
    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();

        if (glfwGetWindowAttrib(main_window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiWindowFlags window_flags = 0;
        ImGui::Begin("My Thing", &test, window_flags);
        ImGui::Text("My Thing! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
        ImGui::Spacing();
        if(ImGui::Button("Swap Spin"))
        {
            render_pipeline->spin_direction = !render_pipeline->spin_direction;
        }

        ImGui::End();

        // ImGui::ShowDemoWindow(&test);

        ImGui::Render();
        // ImDrawData* main_draw_data = ImGui::GetDrawData();
        
        render_pipeline->draw_frame();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void Application::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    render_pipeline->cleanup();
}