#pragma once
#include <GLFW/glfw3.h>
#include "../../../external/imgui_test/imgui.h"
#include "../../../external/imgui_test/imgui_impl_glfw.h"
#include "../../../external/imgui_test/imgui_impl_vulkan.h"
// #include "../../debugger/debugger.cpp"
#include "../../renderer/device/vulkan/device.cpp"
#include "../../renderer/render_pipeline.cpp"
// #include "../../engine/entity_manager/entity_manager.h"

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

void init_imgui(GLFWwindow* main_window, RenderPipeline* render_pipeline)
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
    init_info.Instance = render_pipeline->my_instance;
    init_info.PhysicalDevice = physical_device;
    init_info.Device = virtual_device;
    init_info.QueueFamily = find_queue_families(physical_device, render_pipeline->my_surface).graphics_family.number;

    init_info.Queue = render_pipeline->device.graphics_queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imgui_descriptor_pool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = find_swap_chain_support(physical_device, render_pipeline->my_surface).surface_capabilities.minImageCount + 1;
    init_info.Allocator = nullptr;
    init_info.PipelineInfoMain.RenderPass = render_pipeline->render_pass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // init_info.CheckVkResultFn = check_vk_result;

    ImGui_ImplVulkan_Init(&init_info);
}


static void imgui_hierarchy_pop_up()
{
    if(ImGui::BeginPopupContextItem("hierarchy_pop_up")){
        ImGui::Text("PopUp");
        if(ImGui::Button("Spawn Object")){
            // if(auto contains = EntityManager::get_entity_names().find("GameObject"); contains != EntityManager::get_entity_names().end())
            // {
            //     std::string name ("GameObject");
            //     for (size_t i = 0; i < 9; i++)
            //     {
            //         if(auto contains = EntityManager::get_entity_names().find(name); contains != EntityManager::get_entity_names().end()){
            //             name.push_back('A');
            //         }else{
            //             EntityManager::add_entity(Entity{}, name);
            //             break;
            //         }
            //     }

            // }else{
            //     EntityManager::add_entity(Entity{}, "GameObject");
            // }

        }
        ImGui::EndPopup();
    }
}

// static void imgui_hierarchy(bool& open, Entity* inspecting)
// {
//     ImGui::Begin("Hierarchy", &open);
//         imgui_hierarchy_pop_up();
//         ImGui::Text("Hierarchy!");
//         ImGui::Spacing();

//         if(ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right)){
//             ImGui::OpenPopup("hierarchy_pop_up");
//         }

//         if(ImGui::TreeNode("Thing"))
//         {
//             // auto entities = EntityManager::get_all_entities();
//             // if(EntityManager::get_all_entities().size() > 0)
//             // {
//             //     for (auto& name : EntityManager::get_entity_names())
//             //     {
//             //         ImGui::PushID(name.second);

//             //         if(ImGui::Button(name.first.c_str())){
//             //             *inspecting = EntityManager::get_all_entities()[name.second];
//             //         }
//             //         ImGui::Spacing();

//             //         ImGui::PopID();
//             //     }
//             // }


//             ImGui::TreePop();
//         }
//     ImGui::End();
// }

void begin_imgui_editor_poll(GLFWwindow* main_window, RenderPipeline* render_pipeline, bool& is_open, float fps, std::vector<char*>& editor_logs)
{
    if (glfwGetWindowAttrib(main_window, GLFW_ICONIFIED) != 0)
    {
        ImGui_ImplGlfw_Sleep(10);
        // continue;
    }

    /**/
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("My Thing", &is_open);
    ImGui::Text("My Thing! (%s) (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
    ImGui::Spacing();

    ImGui::Text("(%f)", ((float)fps));
    ComponentSystem cameras = *get_component_system(0);

    for (size_t i = 0; i < cameras.amount; i++)
    {
        ImGui::DragFloat3("Camera Position", &static_cast<CameraComponent*>((void*)cameras.components)[i].transform.position.x, 0.1f);
        ImGui::DragFloat3("Camera Rotation", &static_cast<CameraComponent*>((void*)cameras.components)[i].transform.rotation.x, 0.1f);
        ImGui::DragFloat("Fov", &static_cast<CameraComponent*>((void*)cameras.components)[i].fov, 0.1f);
    }

    // imgui_hierarchy(is_open, inspecting);

    for (uint16_t i = 0; i < render_pipeline->to_render.size(); i++)
    {
        ImGui::PushID(i);

        ImGui::Text("Transform");
        ImGui::Spacing();

        ImGui::PopID();
    }

    ImGui::End();

    ImGui::Begin("Inspector");

        // for(TempID id : inspecting->components){
        //     ImGui::PushID(id.type);
        //     inspect(id.type, id.index);
        //     ImGui::Spacing();
        //     ImGui::PopID();
        // }

    ImGui::End();


    ImGui::Begin("Console");
        Debug::get_all_logs(editor_logs);
        ImGui::BeginChild("Logs");
            for (size_t i = 0; i <  editor_logs.size(); i++)
            {
                ImGui::Text("(%s)", editor_logs[i]);
            }

        ImGui::EndChild();
    ImGui::End();
}

void end_imgui_editor_poll()
{
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

void clean_imgui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
