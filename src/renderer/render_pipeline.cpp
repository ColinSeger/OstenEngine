#pragma once
#include <cstddef>
#include <cstdint>
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
#include "shaders/shaders.h"
#include "../additional_things/arena.h"
#include "render_passes/render_passes.h"

struct RenderData
{
    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];

    std::vector<RenderDescriptors> render_descriptors;
    uint32_t descriptor_usage = 0;
};

struct RenderPipeline
{
    SwapChainImages swap_chain_images = {};
    OffScreenImage offscreen_image;

    //Device manager
    Device device;

    RenderData render_data;

    //TODO move out of class
    VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];

    SwapChain swap_chain = {};

    //The Vulkan instance
    VkInstance my_instance = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;

    VkPipeline graphics_pipeline;
    VkPipeline shadow_pipeline;

    VkSurfaceKHR my_surface;

    VkCommandPool command_pool;

    VkDescriptorPool descriptor_pool;

    VkRenderPass render_pass; //TODO MOVE

    //RenderPipeline(const VkExtent2D screen_size, VkInstance instance, VkSurfaceKHR surface, MemArena& memory_arena);
    //~RenderPipeline();

    //TEMP
    // SHADOWPASS
    ShadowPass shadow_pass;
    VkPipelineLayout shadow_pipe_layout;
    VkDescriptorSetLayout shadow_layout;

    int32_t draw_frame(CameraComponent camera, VkDescriptorSet& imgui_texture, MemArena& memory_arena);
};

static void setup_render_pipeline(VkDevice virtual_device, VkRenderPass render_pass, VkPipelineLayout pipeline_layout, VkPipeline* graphics_pipeline, MemArena& memory_arena){
    //Move this later
    ShaderMemoryIndexing vertex_shader = load_shader("src/renderer/shaders/vert.spv", memory_arena);
    ShaderMemoryIndexing fragment_shader = load_shader("src/renderer/shaders/frag.spv", memory_arena);

    VkPipelineShaderStageCreateInfo vertex_stage_info = create_shader(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT, virtual_device, memory_arena);
    VkPipelineShaderStageCreateInfo fragment_state_info = create_shader(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT, virtual_device, memory_arena);
    free_arena(memory_arena, vertex_shader.arena_index);
    free_arena(memory_arena, fragment_shader.arena_index);//Tecnically only need one of these

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
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex = -1; // Optional
    pipeline_info.renderPass = render_pass;

    VkResult result = vkCreateGraphicsPipelines(virtual_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, graphics_pipeline);

    if(result != VK_SUCCESS){
        throw "Failed to create pipeline";
    }

    vkDestroyShaderModule(virtual_device, fragment_state_info.module, nullptr);
    vkDestroyShaderModule(virtual_device, vertex_stage_info.module, nullptr);
}

void setup_shadow_pipe(VkDevice virtual_device, VkPipelineLayout pipeline_layout, VkPipeline* shadow_pipeline, VkRenderPass shadow_pass, MemArena& memory_arena){
    ShaderMemoryIndexing vertex_shader = load_shader("src/renderer/shaders/quad.vert.spv", memory_arena);
    //ShaderMemoryIndexing fragment_shader = load_shader("src/renderer/shaders/quad.frag.spv", memory_arena);

    VkPipelineShaderStageCreateInfo vertex_stage_info = create_shader(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT, virtual_device, memory_arena);
    //VkPipelineShaderStageCreateInfo fragment_state_info = create_shader(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT, virtual_device, memory_arena);
    free_arena(memory_arena, vertex_shader.arena_index);
    //free_arena(memory_arena, fragment_shader.arena_index);//Tecnically only need one of these

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_stage_info};
    constexpr uint8_t shader_amount = sizeof(shader_stages) / sizeof(shader_stages[0]);

    VkVertexInputBindingDescription binding_description = get_binding_description();
    VertexAttributes attribute_descriptions = get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = 3;
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.array;

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//For if you want to draw wireframe
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_TRUE;//Things that might be good for shadow mapping
    rasterizer.depthBiasConstantFactor = 1.25f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 1.75f; // Optional

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
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
    pipeline_info.stageCount = shader_amount;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = 0;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex = -1; // Optional
    pipeline_info.renderPass = shadow_pass;

    VkResult s_result = vkCreateGraphicsPipelines(virtual_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, shadow_pipeline);

    //vkDestroyShaderModule(virtual_device, fragment_state_info.module, nullptr);
    vkDestroyShaderModule(virtual_device, vertex_stage_info.module, nullptr);
}

void restart_swap_chain(RenderPipeline& render_pipeline, VkExtent2D screen_size, MemArena& memory_arena)
{
    vkDeviceWaitIdle(render_pipeline.device.virtual_device);

    if(render_pipeline.swap_chain_images.image_amount > 0){
        clean_swap_chain(render_pipeline.device.virtual_device, render_pipeline.swap_chain, render_pipeline.swap_chain_images, memory_arena);

        vkDestroyCommandPool(render_pipeline.device.virtual_device, render_pipeline.command_pool, nullptr);

        create_swap_chain(render_pipeline.device, screen_size, render_pipeline.my_surface, render_pipeline.swap_chain, memory_arena);
        create_swap_chain_images(render_pipeline.device, render_pipeline.swap_chain, render_pipeline.my_surface, render_pipeline.swap_chain_images, memory_arena);

    }else{
        create_swap_chain(render_pipeline.device, screen_size, render_pipeline.my_surface, render_pipeline.swap_chain, memory_arena);
        create_swap_chain_images(render_pipeline.device, render_pipeline.swap_chain, render_pipeline.my_surface, render_pipeline.swap_chain_images, memory_arena);
        if(create_render_pass(&render_pipeline.render_pass, render_pipeline.swap_chain.swap_chain_image_format, render_pipeline.device) != VK_SUCCESS) throw "failed to create renderpass";
    }
    render_pipeline.command_pool = CommandBuffer::create_command_pool(render_pipeline.device, render_pipeline.my_surface, memory_arena);

    render_pipeline.swap_chain_images.depth_image_view = create_depth_resources(render_pipeline.device, render_pipeline.swap_chain.screen_extent, render_pipeline.swap_chain_images.depth_image_memory, render_pipeline.swap_chain_images.depth_image);

    VkResult frambuffer_status = create_frame_buffers(render_pipeline.swap_chain_images, render_pipeline.device.virtual_device, render_pipeline.render_pass, render_pipeline.swap_chain_images.depth_image_view, render_pipeline.swap_chain.screen_extent, memory_arena);

    if(frambuffer_status != VK_SUCCESS){
        throw "FrameBuffers could not be created";
    }

    CommandBuffer::create_command_buffers(render_pipeline.command_buffers, render_pipeline.device.virtual_device, render_pipeline.command_pool, MAX_FRAMES_IN_FLIGHT);
}

static void create_sync_objects(VkDevice virtual_device, RenderData* render_pipe)
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if(vkCreateSemaphore(virtual_device, &semaphore_info, nullptr, &render_pipe->image_available_semaphores[i]) != VK_SUCCESS){
            throw "Failed To Create Semaphore";
        }
        if(vkCreateSemaphore(virtual_device, &semaphore_info, nullptr, &render_pipe->render_finished_semaphores[i]) != VK_SUCCESS){
            throw "Failed To Create Semaphore";
        }
        if(vkCreateFence(virtual_device, &fence_info, nullptr, &render_pipe->in_flight_fences[i]) != VK_SUCCESS){
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

        Transform transform = static_cast<TransformComponent*>(get_component_by_id(transform_system, render_component->transform_id))->transform;

        mat4_t model = Transformations::get_model_matrix(transform);

        UniformBufferObject uniform_buffer{
            model,
            view_matrix,
            projection
        };

        memcpy(to_render[render_component->descriptor_id].uniform_buffers_mapped[current_image], &uniform_buffer, sizeof(uniform_buffer));
    }
}

static void update_uniform_buffer_light(const vec3_t camera, const uint8_t current_image, const float aspect_ratio, RenderDescriptors* to_render) {
    //Aspect Ratio =  swap_chain.screen_extent.width / (float) swap_chain.screen_extent.height
    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    ComponentSystem* render =  get_component_system(RENDER);
    Transform test{};
    test.position = camera;
    test.rotation = {0,1.3f,0};
    test.scale = {1,1,1};
    vec3_t forward_vector =  v3_add(camera, Transformations::forward_vector(test));

    mat4_t view_matrix = m4_look_at(camera, forward_vector, {0, 0, 1});
    mat4_t projection = m4_perspective_matrix(45.f, aspect_ratio, 1.f, 2000.0f);

    for (size_t render_index = 0; render_index < render->amount; render_index++)
    {
        RenderComponent* render_component = reinterpret_cast<RenderComponent*>(get_component_by_id(render, render_index));

        Transform transform = static_cast<TransformComponent*>(get_component_by_id(transform_system, render_component->transform_id))->transform;

        mat4_t model = Transformations::get_model_matrix(transform);

        UniformBufferObject uniform_buffer{
            model,
            view_matrix,
            projection
        };

        memcpy(to_render[render_component->descriptor_id].uniform_buffers_mapped[current_image], &uniform_buffer, sizeof(uniform_buffer));
    }
}

RenderPipeline RenderPipeline(const VkExtent2D screen_size, VkInstance instance, VkSurfaceKHR surface, MemArena& memory_arena)
{
    struct RenderPipeline result{};

    result.my_instance = instance;
    result.my_surface = surface;
    create_device(result.device, instance, surface, memory_arena);

    restart_swap_chain(result, screen_size, memory_arena);

    create_forward_descriptor_set_layout(result.device.virtual_device, &result.descriptor_set_layout);
    create_shadow_descriptor_layout(result.device.virtual_device, &result.shadow_layout);

    create_uniform_buffers(result.render_data.render_descriptors.data(), result.render_data.render_descriptors.size(), result.device);
    create_descriptor_pool(result.descriptor_pool, result.device.virtual_device, 100);

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &result.descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

    if(vkCreatePipelineLayout(result.device.virtual_device, &pipeline_layout_info, nullptr, &result.pipeline_layout) != VK_SUCCESS){
        throw "Failed to create pipeline";
    }
    pipeline_layout_info.pSetLayouts = &result.shadow_layout;
    if(vkCreatePipelineLayout(result.device.virtual_device, &pipeline_layout_info, nullptr, &result.shadow_pipe_layout) != VK_SUCCESS){
        throw "Failed to create pipeline";
    }
    create_offscreen_framebuffer(result.device, {1024, 1024}, &result.shadow_pass);

    setup_shadow_pipe(result.device.virtual_device, result.shadow_pipe_layout, &result.shadow_pipeline, result.shadow_pass.render_pass, memory_arena);

    setup_render_pipeline(result.device.virtual_device, result.render_pass, result.pipeline_layout, &result.graphics_pipeline, memory_arena);

    create_sync_objects(result.device.virtual_device, &result.render_data);
    //create_offscreen_image(device, screen_size, render_pass, swap_chain_images.depth_image_view);
    return result;
}

void render_cleanup(struct RenderPipeline& pipeline, MemArena& memory_arena)
{
    vkDeviceWaitIdle(pipeline.device.virtual_device);

    for(Model model : loaded_models){
        vkDestroyBuffer(pipeline.device.virtual_device, model.index_buffer, nullptr);
        vkDestroyBuffer(pipeline.device.virtual_device, model.vertex_buffer, nullptr);
    }

    for(int i = 0; i < loaded_textures.size(); i++){
        vkDestroyImageView(pipeline.device.virtual_device, loaded_textures[i].image_view, nullptr);
        vkDestroyImage(pipeline.device.virtual_device, loaded_textures[i].texture_image, nullptr);
    }

    clean_swap_chain(pipeline.device.virtual_device, pipeline.swap_chain, pipeline.swap_chain_images, memory_arena);
    vkDestroyPipeline(pipeline.device.virtual_device, pipeline.graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(pipeline.device.virtual_device, pipeline.pipeline_layout, nullptr);
    vkDestroyRenderPass(pipeline.device.virtual_device, pipeline.render_pass, nullptr);

    vkDestroyDescriptorPool(pipeline.device.virtual_device, pipeline.descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(pipeline.device.virtual_device, pipeline.descriptor_set_layout, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(pipeline.device.virtual_device, pipeline.render_data.image_available_semaphores[i], nullptr);
        vkDestroySemaphore(pipeline.device.virtual_device, pipeline.render_data.render_finished_semaphores[i], nullptr);
        vkDestroyFence(pipeline.device.virtual_device, pipeline.render_data.in_flight_fences[i], nullptr);
    }

    vkDeviceWaitIdle(pipeline.device.virtual_device);

    vkDestroyCommandPool(pipeline.device.virtual_device, pipeline.command_pool, nullptr);
}

static void swap_draw_frame(VkCommandBuffer& command_buffer, std::vector<RenderDescriptors> descriptors, VkPipelineLayout pipeline_layout, const uint8_t frame){
    constexpr VkDeviceSize offsets[] = {0};

    if(loaded_models.size() <= 0){
        return;
    }

    ComponentSystem* render =  get_component_system(RENDER);

    for (int i = 0; i < render->amount; i++) {
        RenderComponent comp = *reinterpret_cast<RenderComponent*>(get_component_by_id(render, i));

        Model model = loaded_models[comp.mesh_id];

        if(model.index_amount > 0){
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &model.vertex_buffer, offsets);

            vkCmdBindIndexBuffer(command_buffer, model.index_buffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptors[comp.descriptor_id].descriptor_sets[frame], 0, nullptr);

            vkCmdDrawIndexed(command_buffer, model.index_amount, 1, 0, 0, 0);
        }
    }
}

void start_shadow_pass(VkCommandBuffer& command_buffer, VkFramebuffer& frame_buffer, VkRenderPass render_pass, const VkExtent2D viewport_extent, VkPipeline shadow_pipe, VkPipelineLayout layout, std::vector<RenderDescriptors> descriptors, const uint8_t frame){
    //Begining of shadow pass
    VkClearValue clear_values[1]{};
    clear_values[0].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = frame_buffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = viewport_extent;
    render_pass_info.clearValueCount = sizeof(clear_values) / sizeof(clear_values[0]);
    render_pass_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(viewport_extent.width);
    viewport.height = static_cast<float>(viewport_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = viewport_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadow_pipe);
    if(loaded_models.size() <= 0){
        vkCmdEndRenderPass(command_buffer);
        return;
    }
    constexpr VkDeviceSize offsets[] = {0};

    ComponentSystem* render =  get_component_system(RENDER);

    for (int i = 0; i < render->amount; i++) {
        RenderComponent comp = *reinterpret_cast<RenderComponent*>(get_component_by_id(render, i));

        Model model = loaded_models[comp.mesh_id];

        if(model.index_amount > 0){
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &model.vertex_buffer, offsets);

            vkCmdBindIndexBuffer(command_buffer, model.index_buffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptors[comp.descriptor_id].shadow_sets[frame], 0, nullptr);

            vkCmdDrawIndexed(command_buffer, model.index_amount, 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(command_buffer);
}

int32_t RenderPipeline::draw_frame(CameraComponent camera, VkDescriptorSet& imgui_texture, MemArena& memory_arena)
{
    static uint8_t current_frame = 0;//TODO Make better

    constexpr vec3_t light = {10.0f, 0, 2};

    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    ComponentSystem* render =  get_component_system(RENDER);

    vkDeviceWaitIdle(device.virtual_device);//TODO have actual solution for this instead of waiting for device idle
    //Solving this wait should bring great benefits

    for (int i = 0; i < render->amount; i++) {
        RenderComponent* comp = (RenderComponent*)get_component_by_id(render, i);
        update_descriptor_set(device.virtual_device, render_data.render_descriptors[comp->descriptor_id], loaded_textures[comp->texture_id].image_view, loaded_textures[comp->texture_id].texture_sampler);
    }

    vkWaitForFences(device.virtual_device, 1, &render_data.in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    static uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(device.virtual_device, swap_chain.swap_chain, UINT64_MAX, render_data.image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result != VK_SUCCESS)
        return result;

    vkResetFences(device.virtual_device, 1, &render_data.in_flight_fences[current_frame]);

    vkResetCommandBuffer(command_buffers[current_frame], 0);

    VkCommandBuffer command_buffer = command_buffers[current_frame];
    CommandBuffer::record_command_buffer(command_buffer);

    update_uniform_buffer_light(light, current_frame, 1, render_data.render_descriptors.data());

    start_shadow_pass(command_buffers[current_frame], shadow_pass.framebuffer, shadow_pass.render_pass, {1024, 1024},shadow_pipeline, shadow_pipe_layout, render_data.render_descriptors, current_frame);

    //vkDeviceWaitIdle(device.virtual_device);//TODO have actual solution for this instead of waiting for device idle
    update_uniform_buffer(camera, current_frame, swap_chain.screen_extent.width / (float) swap_chain.screen_extent.height, render_data.render_descriptors.data());


    start_render_pass(command_buffer, static_cast<VkFramebuffer*>(memory_arena[swap_chain_images.swap_chain_frame_buffers])[image_index], render_pass, swap_chain.screen_extent);
   // start_render_pass(command_buffer, *offscreen_image.swap_chain_frame_buffers, render_pass, swap_chain.screen_extent);
    bind_pipeline(command_buffer, graphics_pipeline, swap_chain.screen_extent);

    Transform render_transform = reinterpret_cast<TransformComponent*>(get_component_by_id(transform_system, camera.transform_id))->transform;

    swap_draw_frame(command_buffer, render_data.render_descriptors, pipeline_layout, current_frame);

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer, nullptr);

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);

    VkSemaphore wait_semaphores[] = {render_data.image_available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {render_data.render_finished_semaphores[current_frame]};

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    VkResult queue_result = vkQueueSubmit(device.graphics_queue, 1, &submit_info, render_data.in_flight_fences[current_frame]);

    if(queue_result != VK_SUCCESS)
        return queue_result;

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

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}
