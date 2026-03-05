#pragma once
#include <GLFW/glfw3.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include "../../../external/imgui_test/imgui.h"
#include "../../../external/imgui_test/imgui_impl_glfw.h"
#include "../../../external/imgui_test/imgui_impl_vulkan.h"
#include "../../renderer/device/vulkan/device.h"
#include "../../engine/message_system/message.h"
#include "../../renderer/render_pipeline.cpp"
#include "../../engine/entity_manager/entity_manager.cpp"
#include "vulkan/vulkan_core.h"

struct UIData{
    RenderPipeline* render_pipe;
    uint32_t* inspecting;
    vec3_t* target_point;
    bool* paused_state;
};

static inline  VkDescriptorPool create_imgui_descriptor_pool(VkDevice virtual_device){
    VkDescriptorPool imgui_pool;

    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          100 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         100 },
        { VK_DESCRIPTOR_TYPE_SAMPLER,                100 },
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 100;
    pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(virtual_device, &pool_info, nullptr, &imgui_pool);
    return imgui_pool;
}

static inline void init_imgui(GLFWwindow* main_window, struct RenderPipeline* render_pipeline, VkInstance instance, HeapStack* memory_arena){
    VkDescriptorPool imgui_descriptor_pool = create_imgui_descriptor_pool(render_pipeline->device.virtual_device);
    VkPhysicalDevice physical_device = render_pipeline->device.physical_device;
    VkDevice virtual_device = render_pipeline->device.virtual_device;

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
    init_info.Instance = instance;
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

    ImGui_ImplVulkan_Init(&init_info);
}

static inline void inspect(uint8_t type, uint16_t id, RenderPipeline* render_pipe, HeapStack* heap_stack){
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

        if(ImGui::BeginCombo("Instance", "")){
            for(uint32_t i = 0; i < render_pipe->model_render_data.renderable_amount; i++){

                std::string name = std::string("RenderAble ");
                char buf[11];
                snprintf(buf, sizeof(buf), "%u", i);
                for(char c : buf){
                    name.push_back(c);
                }
                if(ImGui::Button(name.c_str())){
                    RenderAble* render_able = (RenderAble*)get_at_index(heap_stack, render_pipe->model_render_data.renderable_memory_index);

                    RenderAble& at_index = render_able[component->instance_id];
                    at_index.instance_amount--;

                    component->instance_id = i;
                    uint16_t index = 0;
                    if(has_component(entities[id], TRANSFORM, &index)){
                        component->transform_id = index;
                    }

                    render_able[i].instance_amount++;
                }
            }
            ImGui::EndCombo();
        }
    }
    break;
    case 3:{
            ImGui::Text("Collider");
            ComponentSystem* collider_system = get_component_system(COLLIDER);
            SimpleColliderComp* collider = ((SimpleColliderComp*)get_component_by_id(collider_system, id));

            ImGui::Text("Collider %i", collider->collision_amount);
            ImGui::Text("Collider %f", collider->collision_range);

        }
    break;
    default:
        break;
    }
}

static float memory_stats[30]{};
static float highest_value = 0;
constexpr uint8_t graph_size = sizeof(memory_stats) / sizeof(memory_stats[0]);

static inline void update_graph(float current){
    if(current > highest_value) highest_value = current;
    float push_value = current;
    for(uint8_t i = 0; i < graph_size; i++){
        float saved = memory_stats[i];
        memory_stats[i] = push_value;
        push_value = saved;
    }
}

static inline void imgui_hierarchy_pop_up(){
    if(ImGui::BeginPopupContextItem("hierarchy_pop_up")){
        ImGui::Text("PopUp");
        if(ImGui::Button("Spawn Object")){
            add_message_f(MessageType::CreateEntity, sizeof("GameObject"), (char*)"GameObject");
        }
        ImGui::EndPopup();
    }
}

static inline void imgui_hierarchy(bool& open, uint32_t& inspecting){
    auto& entities = get_all_entities();
    ImGui::Begin("Hierarchy", &open);
        imgui_hierarchy_pop_up();
        ImGui::Text("Hierarchy!");
        ImGui::Spacing();

        if(ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right)){
            ImGui::OpenPopup("hierarchy_pop_up");
        }

        if(ImGui::TreeNode("Tree"))
        {

            if(entities.size() > 0)
            {
                for (auto& name : get_entity_names())
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

static InstanceData selected_assets;
static inline void show_loaded_assets(RenderPipeline* render_pipe, HeapStack* heap_stack){
    ImGui::Text("Loaded Models");

    for (auto const& value : loaded_model_index){
        ImGui::PushID(value.second);
        if(ImGui::Button(value.first.c_str()) && value.first[value.first.size()-1] == 'j'){
            add_message_f(MessageType::SerializeOBJ, value.first.size(), (char*)value.first.c_str());
        }
        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::Text("Loaded Textures");

    for (uint32_t i = 0; i < texture_amount; i++){
        ImGui::PushID(i);
        ImGui::Button(texture_storage[i].name);
        ImGui::Spacing();
        ImGui::PopID();
    }

    if(ImGui::Button("Create RenderAble")){
        add_message_f(MessageType::CreateRenderable, sizeof(InstanceData), (char*)&selected_assets);
    }
    if(ImGui::BeginCombo("Models", "")){
        for (auto const& value : loaded_model_index){
            if (ImGui::Button(value.first.c_str())){
                selected_assets.model_index = value.second;
            }
        }
        ImGui::EndCombo();
    }
    // if(ImGui::BeginCombo("Textures", "")){
    //     for (auto const& value : loaded_textures_index)
    //     {
    //         if (ImGui::Button(value.first.c_str()))
    //         {
    //             selected_assets.texture_index = value.second;
    //         }
    //     }
    //     ImGui::EndCombo();
    // }
    ImGui::InputInt("Capacity", &selected_assets.capacity);
    ImGui::Text("Existing RenderAble");
    for(uint32_t i = 0; i < render_pipe->model_render_data.renderable_amount; i++){
        RenderAble* renderable = get_renderable(&render_pipe->model_render_data, i, heap_stack);

        ImGui::Text("Texture %i", renderable->texture_index);
        ImGui::Text("Model %i", renderable->model_index);
    }
}

static inline void begin_imgui_editor_poll(GLFWwindow* main_window, struct UIData* ui_data, bool& is_open, float fps, HeapStack* heap_stack){
    vec3_t& target_point = *ui_data->target_point;

    if (glfwGetWindowAttrib(main_window, GLFW_ICONIFIED) != 0){
        ImGui_ImplGlfw_Sleep(10);
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Window", &is_open);

    ImGui::Text("Fps: (%f)", ((float)fps));

    ImGui::PlotLines("Memory Usage", memory_stats, graph_size, 0, nullptr, 0, highest_value  * 1.5f, {100, 100});

    if(ImGui::Button("Pause/Start")){
        *ui_data->paused_state = !*ui_data->paused_state;
    }

    ComponentSystem cameras = *get_component_system(CAMERA);

    for (size_t i = 0; i < cameras.amount; i++)
    {
        ImGui::PushID(i);
        ComponentSystem* transform_system = get_component_system(TRANSFORM);
        CameraComponent* camera = (CameraComponent*)get_component_by_id(&cameras, i);

        Transform& camera_transform = reinterpret_cast<TransformComponent*>(get_component_by_id(transform_system, camera->transform_id))->transform;
        ImGui::DragFloat3("Camera Position", &camera_transform.position.x, 0.1f);
        ImGui::DragFloat3("Camera Rotation", &camera_transform.rotation.x, 0.1f);
        ImGui::DragFloat("Fov", &camera->field_of_view, 0.1f);
        ImGui::PopID();
    }

    ImGui::DragFloat3("Camera Position", &target_point.x, 0.1f);

    show_loaded_assets(ui_data->render_pipe, heap_stack);

    ImGui::Begin("Console");
        std::string* editor_logs = Debug::get_all_logs();
        ImGui::BeginChild("Logs");
            for (size_t i = 0; i <  Debug::logs_size(); i++)
            {
                ImGui::Text("%s", editor_logs[i].c_str());
            }
        ImGui::EndChild();

    ImGui::End();

    auto& entities = get_all_entities();

    if(entities.empty()){
        ImGui::End();
        return;
    }
    imgui_hierarchy(is_open, *ui_data->inspecting);

    ImGui::End();
    ImGui::Begin("Inspector");
        for (auto& entity : get_entity_names()){
            if(entity.second == entities[*ui_data->inspecting].id){
                char buffer[64] = {};
                for (size_t i = 0; i < entity.first.length(); i++) {
                    buffer[i] = entity.first[i];
                }
                ImGui::InputText("Name" , buffer , 64, 0);
                //EntityManager::rename_entity(entity.first, std::string(buffer));
            }
        }

        for(uint16_t i = 0; i < entities[*ui_data->inspecting].component_amount; i++){
            ImGui::PushID(entities[*ui_data->inspecting].components[i].type);
            inspect(entities[*ui_data->inspecting].components[i].type, entities[*ui_data->inspecting].components[i].index, ui_data->render_pipe, heap_stack);
            ImGui::Spacing();
            ImGui::PopID();
        }
        if(ImGui::BeginPopupContextItem("components_pop_up")){
            ImGui::Text("Components");
            if(ImGui::Button("Add Transform")){
                TempID transform{
                    static_cast<uint16_t>(add_transform()),
                    static_cast<uint16_t>(TRANSFORM)
                };
                //entities[inspecting].components.emplace_back(transform);
                add_component(entities[*ui_data->inspecting], transform);
                // inspecting = &EntityManager::get_all_entities()[inspecting->id];
            }
            if(ImGui::Button("Add Render Component")){
                RenderAble* rendera = get_renderable(&ui_data->render_pipe->model_render_data, 0, heap_stack);
                uint16_t index = 0;
                if(has_component(entities[*ui_data->inspecting], TRANSFORM, &index)){
                    TempID render{
                        (uint16_t)(add_render_component(0, index)),
                        (uint16_t)(RENDER)
                    };
                    rendera->instance_amount++;

                    add_component(entities[*ui_data->inspecting], render);
                }

                //entities[inspecting].components.emplace_back(render);
            }
            ImGui::EndPopup();
        }
        if(ImGui::Button("Add Component"))
        {
            ImGui::OpenPopup("components_pop_up");
        }

    ImGui::End();
}

static inline void end_imgui_editor_poll()
{
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}
