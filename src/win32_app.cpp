#pragma once
#include "common_includes.h"
#define COMMON_INCLUDES
#include <GLFW/glfw3.h>
#include "renderer/render_pipeline.h"
#include "engine/entity_manager/entity_manager.h"
#include "editor/file_explorer/file_explorer.h"
#include "engine/entity_manager/entity_system.h"
#include "engine/entity_manager/components.h"
#include "editor/UI/imgui.cpp"

struct OstenEngine
{
    GLFWwindow* main_window = nullptr;
    const char* application_name = nullptr;
    RenderPipeline* render_pipeline = nullptr;

    std::vector<char*> logs;

    std::vector<System> systems;

    FileExplorer file_explorer;
    Entity inspecting;

    bool resized = false;
    static void resize_callback(GLFWwindow* main_window, int width, int height) {
        auto app = reinterpret_cast<OstenEngine*>(glfwGetWindowUserPointer(main_window));
        app->resized = true;
    }


    OstenEngine(const int width, const int height, const char* name);

    ~OstenEngine();
    void main_game_loop();

    void move_camera(double delta_time);

    void cleanup();
};

OstenEngine::OstenEngine(const int width, const int height, const char* name) : application_name { name }
{
    if(!glfwInit()){
        puts("glfwInit failed");
        assert(false && "GLFW Failed to open");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);
    glfwSetWindowUserPointer(main_window, this);
    glfwSetFramebufferSizeCallback(main_window, resize_callback);

    #ifdef NDEBUG
        const std::vector<const char*> validation_layers = {};
    #else
        const std::vector<const char*> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
    #endif
    uint32_t glfw_extention_count = 0;

    // //Gets critical extensions
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extention_count);

    VkInstance instance = Instance::create_instance(application_name, glfw_extention_count, glfw_extensions , validation_layers);
    VkSurfaceKHR surface;

    VkResult result = glfwCreateWindowSurface(instance, main_window, nullptr, &surface);

    if(result != VK_SUCCESS){
        assert(false && "Failed to create surface");
    }

    render_pipeline = new RenderPipeline(width, height, application_name, instance, surface, validation_layers);

    init_imgui(main_window, render_pipeline);

    file_explorer = init_file_explorer();
/**/

}

OstenEngine::~OstenEngine()
{
    cleanup();
}

void OstenEngine::move_camera(double delta_time)
{
    // float camera_speed = 500;
    // if(ImGui::IsKeyDown(ImGuiKey_W)){
    //     render_pipeline->camera_location.position -= Transformations::forward_vector(render_pipeline->camera_location) * delta_time * camera_speed;
    // }
    // if(ImGui::IsKeyDown(ImGuiKey_S)){
    //     render_pipeline->camera_location.position += Transformations::forward_vector(render_pipeline->camera_location)  * delta_time * camera_speed;
    // }
}

void OstenEngine::main_game_loop()
{
    bool test = true;
    static auto start_time = std::chrono::high_resolution_clock::now();
    double frames = 0;

    // VkDescriptorSet image = ImGui_ImplVulkan_AddTexture(render_pipeline->texture_sampler, render_pipeline->last_frame, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    int entity_to_delete = 0;

    double fps = 0;

    auto last_tick = std::chrono::high_resolution_clock::now();
    inspecting = Entity{};
    inspecting.components.push_back({0, 0});

    create_camera_system(1);
    create_transform_system(10);
    inspecting.components.push_back(TempID{add_transform(), 1});

    while(!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();

        auto current_time = std::chrono::high_resolution_clock::now();
        double delta_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - last_tick).count();
        double frame_time = std::chrono::duration<double, std::chrono::seconds::period>(current_time - start_time).count();

        for (System system : systems)
        {
            system.run_system(system);
        }

        if(frame_time > 1)
        {
            fps = frames / frame_time;
            start_time = current_time;
            frames = 0;
        }

        begin_imgui_editor_poll(main_window, render_pipeline, test, fps, logs, &inspecting);
        //ImGui::DockSpaceOverViewport();
        start_file_explorer(file_explorer, render_pipeline);

        end_file_explorer();



        // ImGui::ShowDemoWindow(&test);

        // ImDrawData* main_draw_data = ImGui::GetDrawData();
        move_camera(delta_time);
        ComponentSystem* cameras = get_component_system(0);
        int32_t result = 0;
        for (size_t i = 0; i < cameras->amount; i++)
        {
            result = render_pipeline->draw_frame(*static_cast<CameraComponent*>(get_component_by_id(cameras, i)));
        }


        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized){
            resized = false;
            int32_t width = 0, height = 0;
            glfwGetFramebufferSize(main_window, &width, &height);
            while (width <= 0 || height <= 0) {
                glfwGetFramebufferSize(main_window, &width, &height);
                glfwWaitEvents();
            }
            render_pipeline->restart_swap_chain(width, height);
        }

        end_imgui_editor_poll();

        frames +=1;
        last_tick = std::chrono::high_resolution_clock::now();
    }
}

void OstenEngine::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    render_pipeline->cleanup();
}
