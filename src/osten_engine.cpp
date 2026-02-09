#pragma once
#include <cstdint>
#include <chrono>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "platform.h"
#include "additional_things/arena.h"
#include "renderer/instance/vulkan/instance.cpp"
#include "renderer/render_pipeline.cpp"
#include "editor/UI/editor_gui.cpp"
#include "editor/file_explorer/file_explorer.cpp"
#include "engine/entity_manager/components.cpp"
#include "engine/message_system/message.h"
#include "../external/math_3d.h"

constexpr size_t KB = 1024;
constexpr size_t MB = KB * 1024;
constexpr size_t GB = MB * 1024;

struct OstenEngine
{
    struct RenderPipeline render_pipeline;

    GLFWwindow* main_window = nullptr;
    VkInstance instance;

    FileExplorer file_explorer;

    HeapStack heap_stack;

    bool resized = false;
    static void resize_callback(GLFWwindow* main_window, int width, int height) {
        auto app = reinterpret_cast<OstenEngine*>(glfwGetWindowUserPointer(main_window));
        app->resized = true;
    }

    OstenEngine(const int width, const int height, const char* name);

    ~OstenEngine();
    void main_game_loop();

    void cleanup();
};

OstenEngine::OstenEngine(const int width, const int height, const char* application_name)
{
    heap_stack = init_mem_arena(128*MB);
    if(!glfwInit()){
        puts("glfwInit failed");
        throw("GLFW Failed to open");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);
    glfwSetWindowUserPointer(main_window, this);
    glfwSetFramebufferSizeCallback(main_window, resize_callback);

    VkExtent2D window_size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    uint32_t glfw_extention_count = 0;

    // //Gets critical extensions
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extention_count);

    WindowExtentions window_extentions{ glfw_extensions, glfw_extention_count };

    //Investigate Why so slow
    Instance::create_instance(instance, application_name, window_extentions, heap_stack);

    VkSurfaceKHR surface;

    VkResult result = glfwCreateWindowSurface(instance, main_window, nullptr, &surface);

    if(result != VK_SUCCESS){
        throw "Failed to create surface";
    }

    result = create_render_pipeline(window_size, instance, surface, this->render_pipeline, heap_stack);

    if(result != VK_SUCCESS){
        throw "Failed to create render pipeline";
    }

    init_imgui(main_window, &render_pipeline, instance, heap_stack);

    file_explorer = init_file_explorer();
}

OstenEngine::~OstenEngine()
{
    cleanup();
}

void OstenEngine::main_game_loop()
{
    bool open_window = true;
    static auto start_time = std::chrono::high_resolution_clock::now();
    double frames = 0;

    double fps = 0;

    auto last_tick = std::chrono::high_resolution_clock::now();

    create_transform_system(10000, &heap_stack);
    create_camera_system(2, &heap_stack);
    create_render_component_system(5000, &heap_stack);

    Message empty_entity{
        0,
        MessageType::CreateEntity,
        (void*)"Test"
    };

    add_message(empty_entity);

    uint32_t inspecting = 0;

    VkDescriptorSet imgui_texture = VK_NULL_HANDLE;

    imgui_texture =
    ImGui_ImplVulkan_AddTexture(
        render_pipeline.shadow_pass.debug_sampler,
        render_pipeline.shadow_pass.image_view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

    procces_all_commands(&render_pipeline, heap_stack);
    int i = 0;
    for (; i < entities.size(); i++) {
        RenderAble* rendera = get_renderable(render_pipeline.model_render_data, 0, heap_stack);
        TempID render{
            (uint32_t)(add_render_component(0, rendera->transform_index + rendera->instance_amount)),
            (uint16_t)(RENDER)
        };
        rendera->instance_amount++;

        TransformComponent* transform = (TransformComponent*)get_component_by_id(get_component_system(TRANSFORM) ,rendera->transform_index + rendera->instance_amount);

        transform->transform.position.y += 1.f * i;

        entities[i].components.emplace_back(render);
    }

    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();

        auto current_time = std::chrono::high_resolution_clock::now();
        //double delta_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - last_tick).count();
        double frame_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - start_time).count();

        handle_message(&render_pipeline, heap_stack);

        if(frame_time > 1)
        {
            update_graph(platform_memory_mb());

            fps = frames / frame_time;
            start_time = current_time;
            frames = 0;
        }

        begin_imgui_editor_poll(main_window, &render_pipeline, open_window, fps, inspecting, heap_stack);
        //ImGui::DockSpaceOverViewport();
        start_file_explorer(file_explorer, &render_pipeline);

        end_file_explorer();
        vkDeviceWaitIdle(render_pipeline.device.virtual_device);
        if(imgui_texture != VK_NULL_HANDLE){
            ImGui::Begin("ShadowMap");
            ImGui::Image(
                (ImTextureID)imgui_texture,
                ImVec2(256, 256)
            );
            ImGui::End();
        }

        ComponentSystem* cameras = get_component_system(0);
        int32_t result = 0;
        //for (size_t i = 0; i < cameras->amount; i++)
        {
            CameraComponent* camera = (CameraComponent*)get_component_by_id(cameras, 0);
            CameraComponent* light_source = (CameraComponent*)get_component_by_id(cameras, 1);
            result = render_pipeline.draw_frame(*camera, imgui_texture, heap_stack, *light_source);

            if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized){
                resized = false;
                int32_t width = 0;
                int32_t height = 0;
                glfwGetFramebufferSize(main_window, &width, &height);
                while (width <= 0 || height <= 0) {
                    glfwGetFramebufferSize(main_window, &width, &height);
                    glfwWaitEvents();
                }
                restart_swap_chain(render_pipeline, VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, heap_stack);
                result = render_pipeline.draw_frame(*camera, imgui_texture, heap_stack, *light_source);
            }
        }

        end_imgui_editor_poll();

        frames +=1;
        last_tick = std::chrono::high_resolution_clock::now();
    }
}

void OstenEngine::cleanup()
{
    VkSurfaceKHR surf = render_pipeline.my_surface;

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplVulkan_Shutdown();
    render_cleanup(render_pipeline, heap_stack);
    vkDestroySurfaceKHR(instance, surf, nullptr);
    vkDestroyInstance(instance, nullptr);
    ImGui::DestroyContext();

    destroy_arena(heap_stack);
}
