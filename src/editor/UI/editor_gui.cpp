#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "../../../external/imgui_test/imgui.h"
#include "../../../external/imgui_test/imgui_impl_glfw.h"
#include "../../../external/imgui_test/imgui_impl_vulkan.h"
#include "../../renderer/device/vulkan/device.cpp"
#include "../../engine/message_system/message.cpp"
#include "../../renderer/render_pipeline.cpp"
#include "../../engine/entity_manager/entity_manager.cpp"

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

void init_imgui(GLFWwindow* main_window, RenderPipeline* render_pipeline, MemArena& memory_arena)
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
    init_info.QueueFamily = find_queue_families(physical_device, render_pipeline->my_surface, memory_arena).graphics_family.number;

    init_info.Queue = render_pipeline->device.graphics_queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imgui_descriptor_pool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = find_swap_chain_support(physical_device, render_pipeline->my_surface, memory_arena).surface_capabilities.minImageCount + 1;
    init_info.Allocator = nullptr;
    init_info.PipelineInfoMain.RenderPass = render_pipeline->render_pass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // init_info.CheckVkResultFn = check_vk_result;

    ImGui_ImplVulkan_Init(&init_info);
}

void inspect(uint8_t type, uint16_t id)
{
    switch (type)
    {
    case 0:{
            ImGui::Text("Camera");
            ComponentSystem* transform_system = get_component_system(TRANSFORM);
            Transform camera_transform = reinterpret_cast<TransformComponent*>(get_component_by_id(transform_system, reinterpret_cast<CameraComponent*>(cameras.components)[0].transform_id))->transform;
            ImGui::DragFloat3("Camera Position", &camera_transform.position.x, 0.1f);
            ImGui::DragFloat3("Camera Rotation", &camera_transform.rotation.x, 0.1f);
            ImGui::DragFloat("Fov", &static_cast<CameraComponent*>(get_component_by_id(&cameras, id))->field_of_view, 0.1f);
        }
        break;
    case 1:
        ImGui::Text("Transform");
        ImGui::DragFloat3("Position", &static_cast<TransformComponent*>(get_component_by_id(&transforms, id))->transform.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &static_cast<TransformComponent*>(get_component_by_id(&transforms, id))->transform.rotation.x, 0.1f);
        ImGui::DragFloat3("Scale", &static_cast<TransformComponent*>(get_component_by_id(&transforms, id))->transform.scale.x, 0.1f);
        break;
    case 2:{
            ImGui::Text("Render Component");
            ComponentSystem* transform_system = get_component_system(TRANSFORM);
            ComponentSystem* render_system = get_component_system(RENDER);
            RenderComponent* component = static_cast<RenderComponent*>(get_component_by_id(render_system, id));
            Transform& render_component_transform = reinterpret_cast<TransformComponent*>(get_component_by_id(transform_system, reinterpret_cast<RenderComponent*>(component)[0].transform_id))->transform;

            ImGui::DragFloat3("Render_Position", &render_component_transform.position.x, 0.1f);
            ImGui::DragFloat3("Render Rotation", &render_component_transform.rotation.x, 0.1f);
            ImGui::DragFloat3("Render Scale", &render_component_transform.scale.x, 0.1f);

            if(ImGui::BeginCombo("Models", "")){
                for (auto const& value : loaded_model_index)
                {
                    if (ImGui::Button(value.first.c_str()))
                    {
                        component->mesh_id = value.second;
                    }
                }
                ImGui::EndCombo();
            }
            if(ImGui::BeginCombo("Textures", "")){
                for (auto const& value : loaded_textures_index)
                {
                    if (ImGui::Button(value.first.c_str()))
                    {
                        component->texture_id = value.second;
                    }
                }
                ImGui::EndCombo();
            }
        }
    break;
    default:
        break;
    }
}

static float memory_stats[30]{};
static float highest_value = 0;
constexpr uint8_t graph_size = sizeof(memory_stats) / sizeof(memory_stats[0]);

static void update_graph(float current)
{
    if(current > highest_value) highest_value = current;
    float push_value = current;
    for(uint8_t i = 0; i < graph_size; i++){
        float saved = memory_stats[i];
        memory_stats[i] = push_value;
        push_value = saved;
    }
}

static void imgui_hierarchy_pop_up()
{
    if(ImGui::BeginPopupContextItem("hierarchy_pop_up")){
        ImGui::Text("PopUp");
        if(ImGui::Button("Spawn Object")){
            Message entity{};
            entity.size = 0;
            entity.type = MessageType::CreateEntity;
            entity.value = (void*)"GameObject";
            add_message(entity);
        }
        ImGui::EndPopup();
    }
}

static void imgui_hierarchy(bool& open, uint32_t& inspecting)
{
    auto& entities = EntityManager::get_all_entities();
    ImGui::Begin("Hierarchy", &open);
        imgui_hierarchy_pop_up();
        ImGui::Text("Hierarchy!");
        ImGui::Spacing();

        if(ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right)){
            ImGui::OpenPopup("hierarchy_pop_up");
        }

        if(ImGui::TreeNode("Thing"))
        {

            if(entities.size() > 0)
            {
                for (auto& name : EntityManager::get_entity_names())
                {
                    ImGui::PushID(name.second);

                    if(ImGui::Button(name.first.c_str())){
                        inspecting = name.second;
                    }
                    ImGui::Spacing();

                    ImGui::PopID();
                }
            }


            ImGui::TreePop();
        }
    ImGui::End();
}

static inline void show_loaded_assets()
{
    ImGui::Text("Loaded Models");

    for (auto const& value : loaded_model_index)
    {
        ImGui::PushID(value.second);
        ImGui::Button(value.first.c_str());
        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::Text("Loaded Textures");

    for (auto const& value : loaded_textures_index)
    {
        ImGui::PushID(value.second);
        ImGui::Button(value.first.c_str());
        ImGui::Spacing();
        ImGui::PopID();
    }
}

void begin_imgui_editor_poll(GLFWwindow* main_window, RenderPipeline* render_pipeline, bool& is_open, float fps, uint32_t& inspecting)
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

    ImGui::PlotLines("Memory Usage", memory_stats, graph_size, 0, nullptr, 0, highest_value  * 1.5f, {100, 100});

    ComponentSystem cameras = *get_component_system(CAMERA);

    for (size_t i = 0; i < cameras.amount; i++)
    {
        ComponentSystem* transform_system = get_component_system(TRANSFORM);
        CameraComponent* camera = (CameraComponent*)get_component_by_id(&cameras, i);

        Transform& camera_transform = reinterpret_cast<TransformComponent*>(get_component_by_id(transform_system, camera->transform_id))->transform;
        ImGui::DragFloat3("Camera Position", &camera_transform.position.x, 0.1f);
        ImGui::DragFloat3("Camera Rotation", &camera_transform.rotation.x, 0.1f);
        ImGui::DragFloat("Fov", &camera->field_of_view, 0.1f);
    }

    show_loaded_assets();

    ImGui::Begin("Console");
        std::vector<std::string> editor_logs = Debug::get_all_logs();
        ImGui::BeginChild("Logs");
            for (size_t i = 0; i <  Debug::logs_size(); i++)
            {
                ImGui::Text("%s", editor_logs[i].c_str());
            }
        ImGui::EndChild();

    ImGui::End();

    auto& entities = EntityManager::get_all_entities();

    if(entities.empty()){
        ImGui::End();
        return;
    }
    imgui_hierarchy(is_open, inspecting);

    ImGui::End();
    ImGui::Begin("Inspector");
        for (auto& entity : EntityManager::get_entity_names())
        {
            if(entity.second == entities[inspecting].id){
                char buffer[64] = {};
                for (int i = 0; i < entity.first.length(); i++) {
                    buffer[i] = entity.first[i];
                }
                ImGui::InputText("Name" , buffer , 64, 0);
                //EntityManager::rename_entity(entity.first, std::string(buffer));
            }
        }

        for(TempID& id : entities[inspecting].components){
            ImGui::PushID(id.type);
            inspect(id.type, id.index);
            ImGui::Spacing();
            ImGui::PopID();
        }
        if(ImGui::BeginPopupContextItem("components_pop_up")){
            ImGui::Text("Components");
            if(ImGui::Button("Add Transform")){
                TempID transform{
                    static_cast<uint32_t>(add_transform()),
                    static_cast<uint16_t>(TRANSFORM)
                };
                entities[inspecting].components.emplace_back(transform);
                // inspecting = &EntityManager::get_all_entities()[inspecting->id];
            }
            if(ImGui::Button("Add Render Component")){
                TempID render{
                    static_cast<uint32_t>(add_render_component(render_pipeline->render_data.descriptor_usage)),
                    static_cast<uint16_t>(RENDER)
                };
                render_pipeline->render_data.descriptor_usage++;
                entities[inspecting].components.emplace_back(render);
                //inspecting = &EntityManager::get_all_entities()[inspecting->id];
            }
            ImGui::EndPopup();
        }
        if(ImGui::Button("Add Component"))
        {
            ImGui::OpenPopup("components_pop_up");
        }

    ImGui::End();
}

void end_imgui_editor_poll()
{
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
