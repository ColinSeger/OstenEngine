#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <fstream>
#include <chrono>
#include "../../external/glm/glm.hpp"
#include "../../external/glm/gtc/matrix_transform.hpp"
#include "instance/vulkan/instance.h"
#include "device/vulkan/device.h"
#include "swap_chain/vulkan/swap_chain.h"
#include "render_data/vulkan/render_data.h"

const uint8_t MAX_FRAMES_IN_FLIGHT = 2;

class RenderPipeline
{
private:
    //Window to render to
    GLFWwindow* main_window = nullptr;
    //The Vulkan instance
    Instance* instance = nullptr;
    //Device manager
    Device* device = nullptr;

    SwapChain* swap_chain = nullptr;

    VkRenderPass render_pass;

    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;

    VkPipeline graphics_pipeline;
    
    VkSurfaceKHR surface;

    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;//Look into how to merge into vertex buffer
    VkDeviceMemory index_buffer_memory;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;

    VkDescriptorPool descriptor_pool;
    std::vector<VkDescriptorSet> descriptor_sets;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    uint8_t current_frame = 0;

    //std::vector<char> load_shader(const std::string& file_name);

    VkShaderModule create_shader(const std::vector<char>& code);

    void shader();

    void create_render_pass();

    void create_sync_objects();

    void restart_swap_chain();

    void create_uniform_buffers();

    void update_uniform_buffer(uint8_t current_image);

    void create_descriptor_sets(std::vector<VkDescriptorSet>& result, VkDescriptorPool& descriptor_pool, VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout);

public:
    RenderPipeline(const int width, const int height, const char* application_name);
    ~RenderPipeline();

    void draw_frame();

    void cleanup();

    GLFWwindow* get_main_window(){ return main_window; }
};
