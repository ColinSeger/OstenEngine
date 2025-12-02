#include "application.h"


static VkDescriptorPool create_imgui_descriptor_pool(VkDevice virtual_device)
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

static void init_imgui(GLFWwindow* main_window, RenderPipeline* render_pipeline)
{
    VkDescriptorPool imgui_descriptor_pool = create_imgui_descriptor_pool(render_pipeline->device.virtual_device);
    VkPhysicalDevice physical_device = render_pipeline->device.physical_device;
    VkDevice virtual_device = render_pipeline->device.virtual_device;

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
    // io.WantCaptureMouse = true;
    
    ImGui_ImplGlfw_InitForVulkan(main_window, true);
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = render_pipeline->instance;
    init_info.PhysicalDevice = physical_device;
    init_info.Device = virtual_device;
    init_info.QueueFamily = find_queue_families(physical_device, render_pipeline->surface).graphics_family.value();

    init_info.Queue = render_pipeline->device.graphics_queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imgui_descriptor_pool;
    init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = find_swap_chain_support(physical_device, render_pipeline->surface).surface_capabilities.minImageCount + 1;
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

    main_window = render_pipeline->main_window;

    init_imgui(main_window, render_pipeline);

    file_explorer = init_file_explorer();
/**/
    
}

Application::~Application()
{
    cleanup();
}

void Application::imgui_hierarchy_pop_up()
{
    if(ImGui::BeginPopupContextItem("hierarchy_pop_up")){
        ImGui::Text("PopUp");
        if(ImGui::Button("Spawn Object")){
            Entity_Manager::add_entity(Entity{});
        }
        ImGui::EndPopup();
    }
}

void Application::imgui_hierarchy(bool& open)
{
    ImGui::Begin("Hierarchy", &open);
        imgui_hierarchy_pop_up();
        ImGui::Text("Hierarchy!");
        ImGui::Spacing();

        if(ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right)){
            ImGui::OpenPopup("hierarchy_pop_up");
        }
        
        if(ImGui::TreeNode("Thing"))
        {
            auto entities = Entity_Manager::get_all_entities();
            for (uint16_t i = 0; i < entities.size(); i++)
            {
                ImGui::PushID(i);

                ImGui::Text("(%i)", entities[i].id);
                // ImGui::InputFloat3("Position",  &render_pipeline->to_render[i].transform.position.x);
                // ImGui::SliderFloat3("Rotation", &render_pipeline->to_render[i].transform.rotation.x, 0, 1);
                // ImGui::InputFloat3("Scale",     &render_pipeline->to_render[i].transform.scale.x);
                ImGui::Spacing();

                ImGui::PopID();
            }
            
            ImGui::TreePop();
        }
    ImGui::End();
}

void Application::move_camera(double delta_time)
{
    float camera_speed = 500;
    if(ImGui::IsKeyDown(ImGuiKey_W)){
        render_pipeline->camera_location.position[0] -= camera_speed * delta_time;
    }
    if(ImGui::IsKeyDown(ImGuiKey_A)){
        render_pipeline->camera_location.position[2] -= camera_speed * delta_time;
    }
    if(ImGui::IsKeyDown(ImGuiKey_S)){
        render_pipeline->camera_location.position[0] += camera_speed * delta_time;
    }
    if(ImGui::IsKeyDown(ImGuiKey_D)){
        render_pipeline->camera_location.position[2] += camera_speed * delta_time;
    }
}

void Application::main_game_loop()
{
    bool test = true;
    static auto start_time = std::chrono::high_resolution_clock::now();
    double frames = 0;

    // VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(render_pipeline->texture_sampler, render_pipeline->last_frame, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    int entity_to_delete = 0;

    double fps = 0;

    auto last_tick = std::chrono::high_resolution_clock::now();

    System sys{};
    Transform rt{};
    uint32_t t = sizeof(rt);
    init_system(sys, t, 50);
    //add_action(sys, &print_transform);
    TransformComponent cop;
    sys.type = cop.id;
    sys.run_system = debug;
    for (size_t i = 0; i < 5; i ++)
    {
        TransformComponent component {};

        component.transform.position.x = i;
        add_component(sys, &component);
    }
    sys.run_system(sys);
    

    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();
        int state = glfwGetKey(main_window, GLFW_KEY_E);

        auto current_time = std::chrono::high_resolution_clock::now();
        double delta_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - last_tick).count();
        double frame_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - start_time).count();

        move_camera(delta_time);
        
        if(frame_time > 1)
        {
            fps = frames / frame_time;
            start_time = current_time;
            frames = 0;             
        }

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
        //ImGui::DockSpaceOverViewport();
        start_file_explorer(file_explorer);

        end_file_explorer();

        ImGui::Begin("My Thing", &test);
            ImGui::Text("My Thing! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
            ImGui::Spacing();

            ImGui::Text("(%f)", ((float)fps));

            ImGui::DragFloat3("camera", &render_pipeline->camera_location.position[0], 0.1f);

            imgui_hierarchy(test);
            ImGui::InputInt("Entity To Delete", &entity_to_delete, sizeof(uint32_t));

            if(ImGui::Button("Remove Entity"))
            {
                Entity_Manager::remove_entity(entity_to_delete);
            }

            if(ImGui::Button("Call Entity"))
            {
                Entity_Manager::print_entities();
            }
            
            for (uint16_t i = 0; i < render_pipeline->to_render.size(); i++)
            {
                ImGui::PushID(i);

                ImGui::Text("Transform");
                ImGui::DragFloat3("Position",  &render_pipeline->to_render[i].transform.position.x, 1.f);
                ImGui::DragFloat3("Rotation",  &render_pipeline->to_render[i].transform.rotation.x, 0.1f);
                ImGui::DragFloat3("Scale",     &render_pipeline->to_render[i].transform.scale.x,    1.f);
                ImGui::Spacing();

                ImGui::PopID();
            }

        ImGui::End();


        ImGui::Begin("Console");
            Debug::get_all_logs(logs);
            ImGui::BeginChild("Logs");
                for (size_t i = 0; i <  logs.size(); i++)
                {
                    ImGui::Text("(%s)", logs[i]);
                }
                
            ImGui::EndChild();
        ImGui::End();

        // ImGui::ShowDemoWindow(&test);

        // ImDrawData* main_draw_data = ImGui::GetDrawData();
        
        render_pipeline->draw_frame();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        frames +=1;

        if(Entity_Manager::get_entity_amount() > render_pipeline->to_render.size()){
            Renderable first_obj;
            first_obj.transform.position =  { -1.0f, 0.0f, 0.0f};
            first_obj.transform.rotation =  { 0.0f, 0.0f, -90.0f};
            first_obj.transform.scale    =  { 1.0f, 1.0f, 1.0f};
            render_pipeline->create_uniform_buffer(first_obj);
            render_pipeline->create_descriptor_set(first_obj);
            render_pipeline->to_render.push_back(first_obj);
        }
        last_tick = std::chrono::high_resolution_clock::now();
    }
}

void Application::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    render_pipeline->cleanup();
}