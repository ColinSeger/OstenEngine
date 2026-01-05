#pragma once
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "../../external/math_3d.h"
#include "device/vulkan/device.cpp"
#include "descriptors/descriptors.cpp"
#include "texture/vulkan/texture.cpp"
#include "swap_chain/vulkan/swap_chain.cpp"
#include "model_loader/model_loader.cpp"
#include "../engine/entity_manager/components.cpp"
#include "../../external/imgui_test/imgui_impl_vulkan.h"

struct RenderPipeline
{
    SwapChainImages swap_chain_images = {};

    //Device manager
    Device device;

    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];

    //TODO move out of class
    VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];

    std::vector<RenderDescriptors> render_descriptors;
    uint32_t descriptor_usage = 0;
    // std::vector<Model> models;

    SwapChain swap_chain = {};

    //The Vulkan instance
    VkInstance my_instance = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;

    VkPipeline graphics_pipeline;

    VkSurfaceKHR my_surface;

    VkCommandPool command_pool;

    VkDescriptorPool descriptor_pool;

    VkRenderPass render_pass; //TODO MOVE
    uint8_t current_frame = 0;//TODO MOVE

    // void restart_swap_chain(int32_t width, int32_t height);

    RenderPipeline(const int width, const int height, VkInstance instance, VkSurfaceKHR surface, const char* const* validation_layers, uint8_t layer_amount);
    ~RenderPipeline();

    int32_t draw_frame(CameraComponent camera);

    // void cleanup();
};

typedef struct
{
    char* chars;
    size_t amount;
} String;

static String load_shader(const char* file_name)
{
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        throw "Failed to load shaders";
    }

    String result;
    result.amount = (size_t) file.tellg();
    result.chars = (char*)malloc(sizeof(char) * result.amount);

    file.seekg(0);
    file.read(result.chars, result.amount);

    file.close();
    return result;
}

static VkShaderModule create_shader(const String& code, VkDevice virtual_device) {
    VkShaderModule shader_result;
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.amount;
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.chars);

    if (vkCreateShaderModule(virtual_device, &create_info, nullptr, &shader_result) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shader_result;
}

static void setup_render_pipeline(VkDevice virtual_device, SwapChain& swap_chain, VkRenderPass render_pass, VkPipelineLayout pipeline_layout, VkPipeline* graphics_pipeline)
{
    //Move this later
    String vertex_shader = load_shader("src/renderer/shaders/vert.spv");
    String fragment_shader = load_shader("src/renderer/shaders/frag.spv");

    VkShaderModule vertex_module = create_shader(vertex_shader, virtual_device);
    VkShaderModule fragment_module = create_shader(fragment_shader, virtual_device);

    VkPipelineShaderStageCreateInfo vertex_stage_info{};
    vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage_info.module = vertex_module;
    vertex_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_state_info{};
    fragment_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_state_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_state_info.module = fragment_module;
    fragment_state_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_stage_info, fragment_state_info};

    VkVertexInputBindingDescription binding_description = get_binding_description();
    VertexAttributes attribute_descriptions = get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(sizeof(attribute_descriptions.array) / sizeof(attribute_descriptions.array[0]));
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.array;

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swap_chain.screen_extent.width;
    viewport.height = (float) swap_chain.screen_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain.screen_extent;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;

    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //For if you want to draw wireframe
    rasterizer.lineWidth = 1.0f;

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    //Things that might be good for shadow mapping
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f; // Optional
    depth_stencil.maxDepthBounds = 1.0f; // Optional
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {}; // Optional
    depth_stencil.back = {}; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    //More settings exist


    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    constexpr VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]);
    dynamic_state.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = pipeline_layout;

    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;

    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex = -1; // Optional

    VkResult result = vkCreateGraphicsPipelines(virtual_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, graphics_pipeline);
    if(result != VK_SUCCESS){
        throw "Failed to create pipeline";
    }

    vkDestroyShaderModule(virtual_device, fragment_module, nullptr);
    vkDestroyShaderModule(virtual_device, vertex_module, nullptr);
    free(vertex_shader.chars);
    free(fragment_shader.chars);
}

static void create_render_pass(VkRenderPass* render_pass, VkFormat swap_chain_format, const Device& device)
{
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription color_attachment{};
    color_attachment.format = swap_chain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = Texture::find_depth_formats(device.physical_device);
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription description_attachments[2] = {color_attachment, depth_attachment};

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub_pass{};
    sub_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub_pass.colorAttachmentCount = 1;
    sub_pass.pColorAttachments = &color_attachment_ref;
    sub_pass.pDepthStencilAttachment = &depth_attachment_ref;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = sizeof(description_attachments) / sizeof(description_attachments[0]);
    render_pass_info.pAttachments = description_attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &sub_pass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if(vkCreateRenderPass(device.virtual_device, &render_pass_info, nullptr, render_pass) != VK_SUCCESS){
        throw "Failed to create RenderPass";
    }
}

void restart_swap_chain(RenderPipeline& render_pipeline, int32_t width, int32_t height)
{
    vkDeviceWaitIdle(render_pipeline.device.virtual_device);

    if(render_pipeline.swap_chain_images.image_amount > 0){
        clean_swap_chain(render_pipeline.device.virtual_device, render_pipeline.swap_chain, render_pipeline.swap_chain_images);

        vkDestroyCommandPool(render_pipeline.device.virtual_device, render_pipeline.command_pool, nullptr);

        create_swap_chain(render_pipeline.device, {width, height}, render_pipeline.my_surface, render_pipeline.swap_chain);
        create_swap_chain_images(render_pipeline.device, render_pipeline.swap_chain, render_pipeline.my_surface, render_pipeline.swap_chain_images);

    }else{
        create_swap_chain(render_pipeline.device, {width, height}, render_pipeline.my_surface, render_pipeline.swap_chain);
        create_swap_chain_images(render_pipeline.device, render_pipeline.swap_chain, render_pipeline.my_surface, render_pipeline.swap_chain_images);
        create_render_pass(&render_pipeline.render_pass, render_pipeline.swap_chain.swap_chain_image_format, render_pipeline.device);
    }
    render_pipeline.command_pool = CommandBuffer::create_command_pool(render_pipeline.device, render_pipeline.my_surface);

    render_pipeline.swap_chain_images.depth_image_view = create_depth_resources(render_pipeline.device, render_pipeline.swap_chain.screen_extent, render_pipeline.swap_chain_images.depth_image_memory, render_pipeline.swap_chain_images.depth_image);

    create_frame_buffers(render_pipeline.swap_chain_images, render_pipeline.device.virtual_device, render_pipeline.render_pass, render_pipeline.swap_chain_images.depth_image_view, render_pipeline.swap_chain.screen_extent);

    CommandBuffer::create_command_buffers(render_pipeline.command_buffers, render_pipeline.device.virtual_device, render_pipeline.command_pool, MAX_FRAMES_IN_FLIGHT);
}

static void create_sync_objects(VkDevice virtual_device, RenderPipeline& render_pipe)
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if(vkCreateSemaphore(virtual_device, &semaphore_info, nullptr, &render_pipe.image_available_semaphores[i]) != VK_SUCCESS){
            throw "Failed To Create Semaphore";
        }
        if(vkCreateSemaphore(virtual_device, &semaphore_info, nullptr, &render_pipe.render_finished_semaphores[i]) != VK_SUCCESS){
            throw "Failed To Create Semaphore";
        }
        if(vkCreateFence(virtual_device, &fence_info, nullptr, &render_pipe.in_flight_fences[i]) != VK_SUCCESS){
            throw "Failed To Create Fence";
        }
    }
}

static void update_uniform_buffer(const CameraComponent& camera, const uint8_t current_image, const float aspect_ratio, RenderDescriptors* to_render) {
    //Aspect Ratio =  swap_chain.screen_extent.width / (float) swap_chain.screen_extent.height
    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    ComponentSystem* render =  get_component_system(RENDER);
    Transform camera_transform = reinterpret_cast<TransformComponent*>(get_component_by_id(transform_system, camera.transform_id))->transform;

    vec3_t forward_vector =  v3_add(camera_transform.position, Transformations::forward_vector(camera_transform));

    mat4_t view_matrix = m4_look_at(camera_transform.position, forward_vector, {0, 0, 1});
    mat4_t projection = m4_perspective_matrix(camera.field_of_view, aspect_ratio, 1.f, 2000.0f);

    for (size_t render_index = 0; render_index < render->amount; render_index++)
    {
        RenderComponent* render_component = reinterpret_cast<RenderComponent*>(get_component_by_id(render, render_index));

        mat4_t model = Transformations::get_model_matrix(static_cast<TransformComponent*>(get_component_by_id(transform_system, render_component->transform_id))->transform);

        UniformBufferObject uniform_buffer{
            model,
            view_matrix,
            projection
        };

        memcpy(to_render[render_component->descriptor_id].uniform_buffers_mapped[current_image], &uniform_buffer, sizeof(uniform_buffer));
    }
}

RenderPipeline::RenderPipeline(const int width, const int height, VkInstance instance, VkSurfaceKHR surface, const char* const* validation_layers, uint8_t layer_amount)
{
    my_instance = instance;
    my_surface = surface;
    create_device(device, instance, surface, validation_layers, layer_amount);

    restart_swap_chain(*this ,width, height);

    create_descriptor_set_layout(device.virtual_device, descriptor_set_layout);

    create_uniform_buffers(render_descriptors.data(), render_descriptors.size(), device);
    create_descriptor_pool(descriptor_pool, device.virtual_device);

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

    if(vkCreatePipelineLayout(device.virtual_device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS){
        throw "Failed to create pipeline";
    }

    setup_render_pipeline(device.virtual_device, swap_chain, render_pass, pipeline_layout, &graphics_pipeline);

    create_sync_objects(device.virtual_device, *this);
}

void cleanup(RenderPipeline& pipeline)
{
    vkDeviceWaitIdle(pipeline.device.virtual_device);

    // ImGui_ImplVulkan_Shutdown();

    for(Model model : loaded_models){
        vkDestroyBuffer(pipeline.device.virtual_device, model.index_buffer, nullptr);
        vkDestroyBuffer(pipeline.device.virtual_device, model.vertex_buffer, nullptr);
    }

    for(int i = 0; i < loaded_textures.size(); i++){
        vkDestroyImageView(pipeline.device.virtual_device, loaded_textures[i].image_view, nullptr);
        vkDestroyImage(pipeline.device.virtual_device, loaded_textures[i].texture_image, nullptr);
    }

    clean_swap_chain(pipeline.device.virtual_device, pipeline.swap_chain, pipeline.swap_chain_images);
    vkDestroyPipeline(pipeline.device.virtual_device, pipeline.graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(pipeline.device.virtual_device, pipeline.pipeline_layout, nullptr);
    vkDestroyRenderPass(pipeline.device.virtual_device, pipeline.render_pass, nullptr);

    vkDestroyDescriptorPool(pipeline.device.virtual_device, pipeline.descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(pipeline.device.virtual_device, pipeline.descriptor_set_layout, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(pipeline.device.virtual_device, pipeline.image_available_semaphores[i], nullptr);
        vkDestroySemaphore(pipeline.device.virtual_device, pipeline.render_finished_semaphores[i], nullptr);
        vkDestroyFence(pipeline.device.virtual_device, pipeline.in_flight_fences[i], nullptr);
    }

    vkDeviceWaitIdle(pipeline.device.virtual_device);

    vkDestroyCommandPool(pipeline.device.virtual_device, pipeline.command_pool, nullptr);

    destroy_device(pipeline.device);

    // vkDestroyInstance(pipeline.my_instance, nullptr);
}

RenderPipeline::~RenderPipeline()
{
    cleanup(*this);
}


static void swap_draw_frame(VkCommandBuffer& command_buffer, RenderDescriptors& render_this, VkPipelineLayout pipeline_layout, Model& model,const uint8_t frame)
{
    // VkBuffer vertex_buffers[] = {model.vertex_buffer};
    VkDeviceSize offsets[] = {0};
    if(model.index_amount > 0){
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &model.vertex_buffer, offsets);

        vkCmdBindIndexBuffer(command_buffer, model.index_buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &render_this.descriptor_sets[frame], 0, nullptr);

        vkCmdDrawIndexed(command_buffer, model.index_amount, 1, 0, 0, 0);
    }
}

int32_t RenderPipeline::draw_frame(CameraComponent camera)
{
    vkWaitForFences(device.virtual_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    static uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(device.virtual_device, swap_chain.swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result != VK_SUCCESS) {
        return result;
    }

    update_uniform_buffer(camera, current_frame, swap_chain.screen_extent.width / (float) swap_chain.screen_extent.height, render_descriptors.data());

    vkResetFences(device.virtual_device, 1, &in_flight_fences[current_frame]);

    vkResetCommandBuffer(command_buffers[current_frame], 0);

    VkCommandBuffer command_buffer = command_buffers[current_frame];
    CommandBuffer::record_command_buffer(command_buffer);

    start_render_pass(command_buffer, swap_chain_images.swap_chain_frame_buffers[image_index], render_pass, swap_chain.screen_extent);

    bind_pipeline(command_buffer, graphics_pipeline, swap_chain.screen_extent);

    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    ComponentSystem* render =  get_component_system(RENDER);
    Transform render_transform = reinterpret_cast<TransformComponent*>(get_component_by_id(transform_system, camera.transform_id))->transform;

    for (int i = 0; i < render->amount; i++) {
        RenderComponent comp = reinterpret_cast<RenderComponent*>(render->components)[i];
        if(loaded_models.size() > 0){
            swap_draw_frame(command_buffer, render_descriptors[comp.descriptor_id], pipeline_layout, loaded_models[comp.mesh_id], current_frame);
        }
    }

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer, nullptr);

    vkCmdEndRenderPass(command_buffer);

    end_render_pass(command_buffer);

    VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    VkResult queue_result = vkQueueSubmit(device.graphics_queue, 1, &submit_info, in_flight_fences[current_frame]);

    if(queue_result != VK_SUCCESS)
    {
        return queue_result;
    }

    VkSwapchainKHR swap_chains[] = {swap_chain.swap_chain};

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr; // Optional


    result = vkQueuePresentKHR(device.present_queue, &present_info);

    vkDeviceWaitIdle(device.virtual_device);//TODO have actual solution for this instead of waiting for device idle
    //There Is a Issue if there are multiple renderables as well
    for (int i = 0; i < render->amount; i++) {
        RenderComponent comp = reinterpret_cast<RenderComponent*>(render->components)[i];
        update_descriptor_set(device.virtual_device, render_descriptors[comp.descriptor_id], loaded_textures[comp.texture_id].image_view, loaded_textures[comp.texture_id].texture_sampler);
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}
