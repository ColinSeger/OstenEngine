#pragma once
#include "../common_includes.h"
#include <vulkan/vulkan_core.h>
#include "descriptors/descriptors.h"
#include <cassert>
#include "device/vulkan/device.h"
#include "swap_chain/vulkan/swap_chain.h"
#include "render_data/vulkan/render_data.h"
#include "texture/vulkan/texture.h"
#include "model_loader/model_loader.h"
#include "renderable.h"
#include "../engine/entity_manager/components.h"


const uint8_t MAX_FRAMES_IN_FLIGHT = 2;

static const char* model_location = "assets/debug_assets/viking.obj";
static const char* texture_location = "assets/debug_assets/viking_room.png";

struct RenderPipeline
{
    //The Vulkan instance
    VkInstance my_instance = VK_NULL_HANDLE;
    //Device manager
    Device device;

    SwapChain swap_chain;

    SwapChainImages swap_chain_images;

    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;

    VkPipeline graphics_pipeline;

    VkSurfaceKHR my_surface;

    //TODO check if these even need to be vector
    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;

    //TODO move out of class
    std::vector<VkCommandBuffer> command_buffers;

    std::vector<Vertex>     vertices;//TODO make better
    std::vector<uint32_t>   indices;

    VkCommandPool command_pool;

    VkDescriptorPool descriptor_pool;
    std::vector<Renderable> to_render;
    std::vector<Model> models;

    uint8_t current_frame = 0;//TODO MOVE
    VkRenderPass render_pass; //TODO MOVE

    VkImageView image_view;//TODO Temporary way to access image
    VkSampler texture_sampler;//TODO Temporary way to access sampler

    // Transform camera_location{2, 0, 0};
    // float fov = 45.f;

    void shader();

    void create_render_pass();

    void create_sync_objects();

    void restart_swap_chain(int32_t width, int32_t height);

    void create_uniform_buffers();

    void update_uniform_buffer(CameraComponent camera, uint8_t current_image);

    RenderPipeline(const int width, const int height, const char* application_name, VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> validation_layers);
    ~RenderPipeline();

    int32_t draw_frame(CameraComponent camera);

    void cleanup();

    void create_uniform_buffer(Renderable& render_this);
};
