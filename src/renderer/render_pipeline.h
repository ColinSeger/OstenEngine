#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <fstream>
#include "instance/vulkan/instance.h"
#include "device/vulkan/device.h"
#include "swap_chain/vulkan/swap_chain.h"

class RenderPipeline
{
private:
    const uint8_t MAX_FRAMES_IN_FLIGHT = 2;

    //Window to render to
    GLFWwindow* main_window = nullptr;
    //The Vulkan instance
    Instance* instance = nullptr;
    //Device manager
    Device* device = nullptr;

    SwapChain* swap_chain = nullptr;

    VkRenderPass render_pass;

    VkPipelineLayout pipeline_layout;

    VkPipeline graphics_pipeline;
    
    VkSurfaceKHR surface;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    uint8_t current_frame = 0;

    std::vector<char> load_shader(const std::string& file_name);

    VkShaderModule create_shader(const std::vector<char>& code);

    void shader();

    void create_render_pass();

    void create_sync_objects();

    void restart_swap_chain();

public:
    RenderPipeline(const int width, const int height, const char* application_name);
    ~RenderPipeline();

    void draw_frame();

    void cleanup();

    GLFWwindow* get_main_window(){ return main_window; }
};
