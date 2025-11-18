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
#include "texture/vulkan/texture.h"
#include "model_loader/model_loader.h"
#include "renderable.h"


const uint8_t MAX_FRAMES_IN_FLIGHT = 2;

class RenderPipeline
{
private:
    //Window to render to
    GLFWwindow* main_window = nullptr;
    //The Vulkan instance
    VkInstance instance = nullptr;
    //Device manager
    Device* device = nullptr;

    SwapChain* swap_chain = nullptr;

    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;

    VkPipeline graphics_pipeline;
    
    VkSurfaceKHR surface;

    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;//TODO Look into how to merge into vertex buffer
    VkDeviceMemory index_buffer_memory;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;

    VkDescriptorPool descriptor_pool;
    

    //TODO check if these even need to be vector
    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    //TODO move out of class
    VkDeviceMemory  depth_image_memory;
    VkImageView     depth_image_view;
    std::vector<VkCommandBuffer> command_buffers;
    //

    std::vector<Vertex>     vertices;//TODO make better
    std::vector<uint32_t>   indices;
    std::vector<Renderable> to_render;
    VkCommandPool command_pool;

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

    VkInstance& get_instance() { return instance; }

    Device* get_device() { return device; }

    VkDescriptorPool& get_descriptor_pool() { return descriptor_pool; }

    VkSurfaceKHR& get_surface() { return surface; }

    SwapChain* get_swap_chain() { return swap_chain; }

    bool spin_direction;

    float spin_x = 0;
    float spin_y = 0;
    float spin_z = 1;
    float scale = 1;

    float camera_thing[3] = {2, 2, 2};

    uint8_t current_frame = 0;//TODO MOVE
    VkRenderPass render_pass; //TODO MOVE
    std::vector<VkDescriptorSet> descriptor_sets;//TODO MOVE
    VkImageView image_view;//TODO Temporary way to access image
    VkSampler texture_sampler;//TODO Temporary way to access sampler

    std::vector<std::string> logs;

    void draw_model(Renderable to_render);
};
