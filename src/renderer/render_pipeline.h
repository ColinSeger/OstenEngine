#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#include "../../external/math_3d.h"
#include "device/vulkan/device.h"
#include "descriptors/descriptors.h"
#include "texture/vulkan/texture.h"
#include "swap_chain/vulkan/swap_chain.h"
//#include "model_loader/model_loader.cpp"
#include "../engine/entity_manager/components.h"
// #include "../../external/imgui_test/imgui_impl_vulkan.h"
#include "shaders/shaders.h"
#include "../additional_things/arena.h"
#include "render_passes/render_passes.h"

//K_LOADER_DEBUG=all LD_PRELOAD=/usr/lib/librenderdoc.so ./build/OstenEngine

typedef struct RenderData{
    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];
    //uint32_t descriptor_usage = 0;
} RenderData;

typedef struct Model{
    uint32_t index_amount;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;//TODO Look into how to merge into vertex buffer
    VkDeviceMemory index_buffer_memory;
} Model;

/**
    This is the render-pipeline which is thing that manages all rendering of this engine there is a lot of issues with it that I want to fix but have not found the time for yet.


*/
struct RenderPipeline{
    SwapChainImages swap_chain_images;

    //Device manager
    Device device;

    RenderData render_data;

    //TODO move out of class
    VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];

    SwapChain swap_chain;

    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;

    VkPipeline graphics_pipeline;

    VkSurfaceKHR my_surface;

    VkCommandPool command_pool;

    VkDescriptorPool descriptor_pool;

    VkRenderPass render_pass;

    //TEMP
    // SHADOWPASS
    //ShadowPass shadow_pass;
    VkDescriptorSetLayout shadow_layout;
    VkDescriptorSetLayout fragment_layout;
    ViewDescriptor camera_descript;

    //ViewDescriptor light;
    LightSources lights;

    FrameDescriptor render_descripts;
    FrameDescriptor texture_descriptor;
    VkDescriptorSetLayout model_set_layout;

    //RenderDescriptors test_descriptor;


    //ViewDescriptor light_test;

    //VkBuffer light_position;

    ModelData model_render_data;

    //int32_t draw_frame(CameraComponent& camera, HeapStack* heap_stack);
};

void render_cleanup(struct RenderPipeline* pipeline, HeapStack* memory_stack)
{
    vkDeviceWaitIdle(pipeline->device.virtual_device);

    // for(Model model : loaded_models){
    //     vkDestroyBuffer(pipeline.device.virtual_device, model.index_buffer, nullptr);
    //     vkDestroyBuffer(pipeline.device.virtual_device, model.vertex_buffer, nullptr);
    // }

    // for(size_t i = 0; i < loaded_textures.size(); i++){
    //     vkDestroyImageView(pipeline.device.virtual_device, loaded_textures[i].image_view, nullptr);
    //     vkDestroyImage(pipeline.device.virtual_device, loaded_textures[i].texture_image, nullptr);
    // }

    clean_swap_chain(pipeline->device.virtual_device, &pipeline->swap_chain, &pipeline->swap_chain_images, memory_stack);
    vkDestroyPipeline(pipeline->device.virtual_device, pipeline->graphics_pipeline, 0);
    vkDestroyPipelineLayout(pipeline->device.virtual_device, pipeline->pipeline_layout, 0);
    vkDestroyRenderPass(pipeline->device.virtual_device, pipeline->render_pass, 0);

    vkDestroyDescriptorPool(pipeline->device.virtual_device, pipeline->descriptor_pool, 0);
    vkDestroyDescriptorSetLayout(pipeline->device.virtual_device, pipeline->descriptor_set_layout, 0);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vkDestroySemaphore(pipeline->device.virtual_device, pipeline->render_data.image_available_semaphores[i], 0);
        vkDestroySemaphore(pipeline->device.virtual_device, pipeline->render_data.render_finished_semaphores[i], 0);
        vkDestroyFence(pipeline->device.virtual_device, pipeline->render_data.in_flight_fences[i], 0);
    }

    vkDeviceWaitIdle(pipeline->device.virtual_device);

    vkDestroyCommandPool(pipeline->device.virtual_device, pipeline->command_pool, 0);
}

static VkResult setup_render_pipeline(VkDevice virtual_device, VkRenderPass render_pass, VkPipelineLayout pipeline_layout, VkPipeline* graphics_pipeline){

    FileData vertex_code = platform_load_entire_file("src/renderer/shaders/vert.spv");
    FileData fragment_code = platform_load_entire_file("src/renderer/shaders/frag.spv");

    VkPipelineShaderStageCreateInfo vertex_stage_info = {};
    VkPipelineShaderStageCreateInfo fragment_state_info = {};

    bool vertex_result = create_shader(vertex_code, VK_SHADER_STAGE_VERTEX_BIT, virtual_device, &vertex_stage_info);
    bool fragment_result = create_shader(fragment_code, VK_SHADER_STAGE_FRAGMENT_BIT, virtual_device, &fragment_state_info);

    platform_free_file(vertex_code);
    platform_free_file(fragment_code);

    if(!vertex_result ||!fragment_result){
        return VK_INCOMPATIBLE_SHADER_BINARY_EXT;
    }

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_stage_info, fragment_state_info};

    VkVertexInputBindingDescription binding_description = get_binding_description();
    VertexAttributes attribute_descriptions = get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = (uint32_t)(sizeof(attribute_descriptions.array) / sizeof(attribute_descriptions.array[0]));
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.array;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//For if you want to draw wireframe
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multi_sampling = {};
    multi_sampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multi_sampling.sampleShadingEnable = VK_FALSE;
    multi_sampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    //More settings exist

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    const VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = (uint32_t)(sizeof(dynamic_states) / sizeof(dynamic_states[0]));
    dynamic_state.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multi_sampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.subpass = 0;
    pipeline_info.renderPass = render_pass;

    VkResult result = vkCreateGraphicsPipelines(virtual_device, VK_NULL_HANDLE, 1, &pipeline_info, 0, graphics_pipeline);

    if(result != VK_SUCCESS){
        return result;
        //"Failed to create pipeline";
    }

    vkDestroyShaderModule(virtual_device, fragment_state_info.module, 0);
    vkDestroyShaderModule(virtual_device, vertex_stage_info.module, 0);
    return VK_SUCCESS;
}

static VkResult setup_shadow_pipe(VkDevice virtual_device, ShadowPass* shadow_pass, LightSources* lights_){
    //ShaderMemoryIndexing vertex_shader = load_shader("src/renderer/shaders/quad.vert.spv", heap_stack);
    FileData vertex_code = platform_load_entire_file("src/renderer/shaders/quad.vert.spv");
    VkPipelineShaderStageCreateInfo vertex_stage_info = {};
    bool vertex_result = create_shader(vertex_code, VK_SHADER_STAGE_VERTEX_BIT, virtual_device, &vertex_stage_info);
    platform_free_file(vertex_code);

    if(!vertex_result){
        return VK_INCOMPATIBLE_SHADER_BINARY_EXT;
    }

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_stage_info};
    const uint8_t shader_amount = sizeof(shader_stages) / sizeof(shader_stages[0]);

    VkVertexInputBindingDescription binding_description = get_binding_description();
    VertexAttributes attribute_descriptions = get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = 3;
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.array;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasConstantFactor = 4.f;
    rasterizer.depthBiasSlopeFactor = 1.75f;

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    const VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]);
    dynamic_state.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo pipeline_info = {};
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
    pipeline_info.layout = lights_->shadow_pipe_layout;
    pipeline_info.subpass = 0;
    pipeline_info.renderPass = lights_->render_pass;

    VkResult result = vkCreateGraphicsPipelines(virtual_device, VK_NULL_HANDLE, 1, &pipeline_info, 0, &lights_->shadow_pipeline);

    vkDestroyShaderModule(virtual_device, vertex_stage_info.module, 0);
    if(result != VK_SUCCESS) return result;
    return VK_SUCCESS;
}

static VkResult restart_swap_chain(struct RenderPipeline* render_pipeline, VkExtent2D screen_size, HeapStack* memory_arena)
{
    vkDeviceWaitIdle(render_pipeline->device.virtual_device);

    if(render_pipeline->swap_chain_images.image_amount > 0){
        clean_swap_chain(render_pipeline->device.virtual_device, &render_pipeline->swap_chain, &render_pipeline->swap_chain_images, memory_arena);

        vkDestroyCommandPool(render_pipeline->device.virtual_device, render_pipeline->command_pool, 0);

        create_swap_chain(&render_pipeline->device, screen_size, render_pipeline->my_surface, &render_pipeline->swap_chain, memory_arena);
        create_swap_chain_images(&render_pipeline->device, &render_pipeline->swap_chain, render_pipeline->my_surface, &render_pipeline->swap_chain_images, memory_arena);

    }else{
        create_swap_chain(&render_pipeline->device, screen_size, render_pipeline->my_surface, &render_pipeline->swap_chain, memory_arena);
        create_swap_chain_images(&render_pipeline->device, &render_pipeline->swap_chain, render_pipeline->my_surface, &render_pipeline->swap_chain_images, memory_arena);

        VkResult render_pass_result = create_render_pass(&render_pipeline->render_pass, render_pipeline->swap_chain.swap_chain_image_format, &render_pipeline->device);
        if(render_pass_result != VK_SUCCESS)
            return render_pass_result;
            //"failed to create renderpass";
    }
    VkResult command_pool = create_command_pool(&render_pipeline->device, render_pipeline->my_surface, memory_arena, &render_pipeline->command_pool);
    if(command_pool != VK_SUCCESS){
        return command_pool;
        //"Failed to create commandpool";
    }

    render_pipeline->swap_chain_images.depth_image_view = create_depth_resources
    (
        &render_pipeline->device,
        render_pipeline->swap_chain.screen_extent,
        &render_pipeline->swap_chain_images.depth_image_memory,
        &render_pipeline->swap_chain_images.depth_image
    );

    VkResult frambuffer_status = create_frame_buffers(&render_pipeline->swap_chain_images, render_pipeline->device.virtual_device, render_pipeline->render_pass, render_pipeline->swap_chain_images.depth_image_view, render_pipeline->swap_chain.screen_extent, memory_arena);

    if(frambuffer_status != VK_SUCCESS){
        return frambuffer_status;
        //"FrameBuffers could not be created";
    }

    VkResult command_buffers = create_command_buffers(render_pipeline->command_buffers, render_pipeline->device.virtual_device, render_pipeline->command_pool, MAX_FRAMES_IN_FLIGHT);
    if(command_buffers != VK_SUCCESS){
        return command_buffers;
        //"Failed to create command buffers";
    }
    return VK_SUCCESS;
}

static VkResult create_sync_objects(VkDevice virtual_device, RenderData* render_pipe){
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult result = vkCreateSemaphore(virtual_device, &semaphore_info, 0, &render_pipe->image_available_semaphores[i]);
        if(result != VK_SUCCESS){
            return result;

            // "Failed To Create Semaphore";
        }
        result = vkCreateSemaphore(virtual_device, &semaphore_info, 0, &render_pipe->render_finished_semaphores[i]);
        if(result != VK_SUCCESS){
            return result;
            //"Failed To Create Semaphore";
        }

        result = vkCreateFence(virtual_device, &fence_info, 0, &render_pipe->in_flight_fences[i]);
        if(result != VK_SUCCESS){
            return result;
            //"Failed To Create Fence";
        }
    }
    return VK_SUCCESS;
}

static void update_view_buffer(Transform transform, ViewDescriptor* view_descriptor, uint8_t current_image, float aspect_ratio, float field_of_view, float range, uint8_t amount){

    vec3_t forward_vector =  v3_add(transform.position, v3_forward_vector(transform));

    mat4_t view_matrix = m4_look_at(transform.position, forward_vector, (vec3_t){0, 0, 1});
    mat4_t projection = m4_perspective_matrix(field_of_view, aspect_ratio, 0.8f, range);
    mat4_t v_matrix = m4_mul(projection, view_matrix);

    memcpy(view_descriptor->uniform_buffers_mapped[current_image], &v_matrix, sizeof(mat4_t) * amount);
}

static void update_view_buffer_light(Transform* transform, ViewDescriptor* view_descriptor, uint8_t current_image, float aspect_ratio, float field_of_view, float range){

    mat4_t v_matrix[MAX_LIGHTS] = {};
    mat4_t projection = m4_perspective_matrix(field_of_view, aspect_ratio, 0.8f, range);
    for(int light = 0; light < MAX_LIGHTS; light++){
        vec3_t forward_vector =  v3_add(transform[light].position, v3_forward_vector(transform[light]));

        mat4_t view_matrix = m4_look_at(transform[light].position, forward_vector, (vec3_t){0, 0, 1});
        v_matrix[light] = m4_mul(projection, view_matrix);
    }

    memcpy(view_descriptor->uniform_buffers_mapped[current_image], v_matrix, sizeof(mat4_t) * MAX_LIGHTS);
}

static inline void update_view_buffer_orthographic(uint32_t transform_id, ViewDescriptor cam_descript, const uint8_t current_image, const float aspect_ratio, float field_of_view, float range){
    //Aspect Ratio =  swap_chain.screen_extent.width / (float) swap_chain.screen_extent.height
    ComponentSystem* transform_system = get_component_system(TRANSFORM);
    Transform camera_transform = ((TransformComponent*)get_component_by_id(transform_system, transform_id))->transform;

    mat4_t view_matrix = m4_look_at(camera_transform.position, (vec3_t){0, 0, 0}, (vec3_t){0, 0, 1});
    mat4_t projection = m4_orthographic_matrix(-10.0f, 10.0f, -10.0f, 10.0f, 0.8f, 2000);
    mat4_t v_matrix = m4_mul(projection, view_matrix);

    memcpy(cam_descript.uniform_buffers_mapped[current_image], &v_matrix, sizeof(mat4_t));
}

static inline void update_uniform_buffer(const uint8_t current_frame, ModelData* to_render, HeapStack* heap_stack) {
    ComponentSystem* transform_system = get_component_system(TRANSFORM);

    for (size_t render_index = 0; render_index < to_render->renderable_amount; render_index++)
    {
        ObjectUBO* model_buffer = get_mapped_uniforms(to_render, heap_stack, render_index, current_frame);
        RenderAble* render_able = get_renderable(to_render, render_index, heap_stack);
        uint16_t* transform_index = (uint16_t*)get_at_index(heap_stack, render_able->transform_index);
        for(uint16_t i = 0; i < render_able->instance_amount; i++){
            Transform transform = ((TransformComponent*)get_component_by_id(transform_system, transform_index[i]))->transform;

            model_buffer[i].model = get_model_matrix(transform);
        }
    }
}

static VkResult create_render_pipeline(const VkExtent2D screen_size, VkInstance instance, VkSurfaceKHR surface, struct RenderPipeline* render_pipeline, HeapStack* heap_stack){
    VkResult result = VK_SUCCESS;
    render_pipeline->my_surface = surface;
    result = create_device(&render_pipeline->device, instance, surface, heap_stack);
    render_pipeline->lights.light_amount = 0;

    if(result != VK_SUCCESS){
        //"Descriptorpool failed to create"
        return result;
    }

    result = restart_swap_chain(render_pipeline, screen_size, heap_stack);

    create_forward_descriptor_set_layout(render_pipeline->device.virtual_device, &render_pipeline->descriptor_set_layout);
    create_shadow_set_layout(render_pipeline->device.virtual_device, &render_pipeline->shadow_layout);

    create_model_set_layout(render_pipeline->device.virtual_device, &render_pipeline->model_set_layout);

    result = create_fragment_layout(render_pipeline->device.virtual_device, &render_pipeline->fragment_layout);

    render_pipeline->model_render_data.object_capacity = (1024 * 50);

    result = init_model_data(&render_pipeline->model_render_data, &render_pipeline->device, heap_stack);

    create_view_uniform_buffer(&render_pipeline->camera_descript, &render_pipeline->device, 1);
    //create_view_uniform_buffer(&render_pipeline.light_test, &render_pipeline.device);
    create_view_uniform_buffer(&render_pipeline->lights.view_descriptor, &render_pipeline->device, MAX_LIGHTS);

    result = create_descriptor_pool(&render_pipeline->descriptor_pool, render_pipeline->device.virtual_device, 100);

    if(result != VK_SUCCESS){
        //"Descriptorpool failed to create"
        return result;
    }

    TextureImage texture = load_texture(&render_pipeline->device, (char*)".png", render_pipeline->command_pool).texture;

    init_light_positions(&render_pipeline->device, &render_pipeline->lights);

    result = create_descriptor_set(render_pipeline->device.virtual_device, &render_pipeline->render_descripts, render_pipeline->descriptor_pool, render_pipeline->descriptor_set_layout, &render_pipeline->camera_descript, &render_pipeline->lights);
    if(result != VK_SUCCESS)
        return result;

    result = create_shadow_sets(render_pipeline->device.virtual_device, &render_pipeline->lights, render_pipeline->descriptor_pool, render_pipeline->shadow_layout);

    if(result != VK_SUCCESS)
        return result;

    VkDescriptorSetLayout forward_layouts[] = { render_pipeline->descriptor_set_layout, render_pipeline->model_set_layout, render_pipeline->fragment_layout};
    uint32_t forward_layout_amount = sizeof(forward_layouts) / sizeof(forward_layouts[0]);

    VkDescriptorSetLayout shadow_layouts[] = { render_pipeline->shadow_layout, render_pipeline->model_set_layout};
    uint32_t shadow_layout_amount = sizeof(shadow_layouts) / sizeof(shadow_layouts[0]);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipeline_layout_info.setLayoutCount = shadow_layout_amount;
    pipeline_layout_info.pSetLayouts = shadow_layouts;

    result = vkCreatePipelineLayout(render_pipeline->device.virtual_device, &pipeline_layout_info, 0, &render_pipeline->lights.shadow_pipe_layout);
    if(result != VK_SUCCESS){
        return result;
        //"Failed to create pipeline";
    }

    create_offscreen_render_pass(&render_pipeline->lights.render_pass, &render_pipeline->device);

    create_shadow_samplers(render_pipeline->device.virtual_device, &render_pipeline->lights.shadow_sampler, &render_pipeline->lights.debug_shadow_sampler);

    for(uint8_t i = 0; i < MAX_LIGHTS; i++){
        create_offscreen_framebuffer(&render_pipeline->device, (VkExtent2D){1024, 1024}, &render_pipeline->lights.shadow_passes[i], &render_pipeline->lights.render_pass);
    }
    create_new_fragment_set(render_pipeline->device.virtual_device, render_pipeline->descriptor_pool, render_pipeline->fragment_layout, &render_pipeline->texture_descriptor, &render_pipeline->lights, &texture);

 //    VkPushConstantRange vertex_constant;
	// vertex_constant.offset = 0;
	// vertex_constant.size = sizeof(uint32_t);
	// vertex_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPushConstantRange fragment_constant = {};
	fragment_constant.offset = 0;
	fragment_constant.size = sizeof(uint32_t) + sizeof(vec3_t);
	fragment_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPushConstantRange push_constants[] = { fragment_constant };

    pipeline_layout_info.setLayoutCount = forward_layout_amount;
    pipeline_layout_info.pSetLayouts = forward_layouts;
    pipeline_layout_info.pPushConstantRanges = push_constants;
    pipeline_layout_info.pushConstantRangeCount = sizeof(push_constants) / sizeof(push_constants[0]);

    result = vkCreatePipelineLayout(render_pipeline->device.virtual_device, &pipeline_layout_info, 0, &render_pipeline->pipeline_layout);

    if(result != VK_SUCCESS){
        return result;
        //"Failed to create pipeline";
    }

    setup_shadow_pipe(render_pipeline->device.virtual_device, &render_pipeline->lights.shadow_passes[0], &render_pipeline->lights);

    setup_render_pipeline(render_pipeline->device.virtual_device, render_pipeline->render_pass, render_pipeline->pipeline_layout, &render_pipeline->graphics_pipeline);

    create_sync_objects(render_pipeline->device.virtual_device, &render_pipeline->render_data);
    return VK_SUCCESS;
}

static vec3_t shader_test = {0.05f, 0.5f, 64.f};

static void swap_draw_frame(VkCommandBuffer* command_buffer, FrameDescriptor* descriptors, FrameDescriptor* textures, ModelData model_data, VkPipelineLayout pipeline_layout, uint8_t frame, HeapStack* heap_stack, vec3_t cam_pos, float lights_amount){
    Model loaded_models[] = {};
    if(sizeof(loaded_models) <= 0 || model_data.renderable_amount <= 0) return;
    return;

    for(uint16_t render_index = 0; render_index < model_data.renderable_amount; render_index++){
        RenderAble* render_data = get_renderable(&model_data, render_index, heap_stack);

        if(render_data->instance_amount <= 0) continue;
        VkDescriptorSet passed_descriptors[] = {descriptors->descriptor_sets[frame], render_data->model_descriptor_sets[frame], textures->descriptor_sets[frame]};
        const size_t descriptor_amount = sizeof(passed_descriptors) / sizeof(passed_descriptors[0]);

        vkCmdBindDescriptorSets(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, descriptor_amount, passed_descriptors, 0, 0);

        Model model = loaded_models[render_data->model_index];

        if(model.index_amount <= 0) return;
        //uint32_t test = 1;
        //vkCmdPushConstants(command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &test);

        vkCmdPushConstants(*command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &render_data->texture_index);
        vkCmdPushConstants(*command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 4, sizeof(vec3_t), &shader_test);
        vkCmdPushConstants(*command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 16, sizeof(vec3_t), &cam_pos);
        vkCmdPushConstants(*command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 28, sizeof(float), &lights_amount);

        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(*command_buffer, 0, 1, &model.vertex_buffer, offsets);

        vkCmdBindIndexBuffer(*command_buffer, model.index_buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(*command_buffer, model.index_amount, render_data->instance_amount, 0, 0, 0);
    }
}


static void start_shadow_pass(VkCommandBuffer* command_buffer, const VkExtent2D viewport_extent, struct LightSources* lights, ModelData model_data, const uint8_t frame, HeapStack* heap_stack){
    //Begining of shadow pass
    VkClearValue clear_values[1] = {};
    clear_values[0].depthStencil = (VkClearDepthStencilValue){1.0f, 0};

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = ((float)viewport_extent.width);
    viewport.height =((float)viewport_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(*command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = viewport_extent;
    vkCmdSetScissor(*command_buffer, 0, 1, &scissor);

    vkCmdBindPipeline(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lights->shadow_pipeline);

    Model loaded_models[0] = {};

    //if(sizeof(loaded_models) <= 0 || model_data.renderable_amount <= 0){
        vkCmdEndRenderPass(*command_buffer);
        return;
        //}
    const VkDeviceSize offsets[] = {0};
    for(uint8_t light_index = 0; light_index < (uint8_t)lights->light_amount; light_index++){

        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = lights->render_pass;
        render_pass_info.framebuffer = lights->shadow_passes[light_index].framebuffer;
        render_pass_info.renderArea.offset = (VkOffset2D){0, 0};
        render_pass_info.renderArea.extent = viewport_extent;
        render_pass_info.clearValueCount = sizeof(clear_values) / sizeof(clear_values[0]);
        render_pass_info.pClearValues = clear_values;

        vkCmdBeginRenderPass(*command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        for(uint16_t render_index = 0; render_index < model_data.renderable_amount; render_index++){
            RenderAble* render_data = get_renderable(&model_data, render_index, heap_stack);

            if(render_data == 0  || render_data->instance_amount <= 0) continue;
            VkDescriptorSet descriptors[] = { lights->lights_desc[light_index].descriptor_sets[frame], render_data->model_descriptor_sets[frame]};
            uint32_t descriptor_amount = sizeof(descriptors) / sizeof(descriptors[0]);

            vkCmdBindDescriptorSets(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lights->shadow_pipe_layout, 0, descriptor_amount, descriptors, 0, 0);

            Model model = loaded_models[render_data->model_index];

            if(model.index_amount > 0){
                vkCmdBindVertexBuffers(*command_buffer, 0, 1, &model.vertex_buffer, offsets);

                vkCmdBindIndexBuffer(*command_buffer, model.index_buffer, 0, VK_INDEX_TYPE_UINT32);

                vkCmdDrawIndexed(*command_buffer, model.index_amount, render_data->instance_amount, 0, 0, 0);
            }
        }
        vkCmdEndRenderPass(*command_buffer);
    }
}

static Transform tasd[MAX_LIGHTS] = {};

int32_t render_frame(struct RenderPipeline* render_pipeline,CameraComponent* camera, HeapStack* heap_stack)
{
    vkDeviceWaitIdle(render_pipeline->device.virtual_device);//Fix Later. Sinks about 300fps
    static uint8_t current_frame = 0;//TODO Make better
    static uint32_t image_index = 0;

    ComponentSystem* trans_sys = get_component_system(TRANSFORM);
    //TransformComponent* t = (TransformComponent*)get_component_by_id(trans_sys, light_source.transform_id);
    TransformComponent* cam = (TransformComponent*)get_component_by_id(trans_sys, camera->transform_id);

    // ImGui::DragFloat3("Shader data (ambient, specular, shininess)", &shader_test.x, 0.01f);

    // ImGui::SliderFloat("Light Amount", &lights.light_amount, 0, 8);

    // for(int i = 0; i < lights.light_amount; i++){
    //     ImGui::PushID(i);
    //     ImGui::DragFloat3("Light Position", &tasd[i].position.x);
    //     lights.light_positions[i].x = tasd[i].position.x;
    //     lights.light_positions[i].y = tasd[i].position.y;
    //     lights.light_positions[i].z = tasd[i].position.z;
    //     lights.light_positions[i].w = 1;

    //     ImGui::DragFloat3("Light R", &tasd[i].rotation.x, 0.001f);
    //     ImGui::DragFloat3("Light S", &tasd[i].scale.x, 0.001f);

    //     ImGui::Spacing();
    //     ImGui::PopID();
    // }

    //update_lights(&lights, lights.light_positions, lights.light_amount);

    update_view_buffer_light(&tasd[0], &render_pipeline->lights.view_descriptor, current_frame, 1, camera->field_of_view, 200.f);

    //update_view_buffer(light_source.transform_id, light_tes, current_frame, 1, light_source.field_of_view, 2000.f);
    update_view_buffer(cam->transform, &render_pipeline->camera_descript, current_frame, render_pipeline->swap_chain.screen_extent.width / (float) render_pipeline->swap_chain.screen_extent.height, camera->field_of_view, 2000.f, 1);
    update_uniform_buffer(current_frame, &render_pipeline->model_render_data, heap_stack);

    vkWaitForFences(render_pipeline->device.virtual_device, 1, &render_pipeline->render_data.in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(render_pipeline->device.virtual_device, render_pipeline->swap_chain.swap_chain, UINT64_MAX, render_pipeline->render_data.image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result != VK_SUCCESS)
        return result;

    vkResetFences(render_pipeline->device.virtual_device, 1, &render_pipeline->render_data.in_flight_fences[current_frame]);

    VkCommandBuffer command_buffer = render_pipeline->command_buffers[current_frame];
    vkResetCommandBuffer(command_buffer, 0);

    if(record_command_buffer(command_buffer) != VK_SUCCESS)
        return result;

    start_shadow_pass(&command_buffer, (VkExtent2D){1024, 1024}, &render_pipeline->lights, render_pipeline->model_render_data, current_frame, heap_stack);

    void* frame_buffer = get_at_index(heap_stack, render_pipeline->swap_chain_images.swap_chain_frame_buffers);

    start_render_pass(command_buffer, ((VkFramebuffer*)frame_buffer)[image_index], render_pipeline->render_pass, render_pipeline->swap_chain.screen_extent);

    bind_pipeline(command_buffer, render_pipeline->graphics_pipeline, render_pipeline->swap_chain.screen_extent);

    //TransformComponent* cam2 = (TransformComponent*)get_component_by_id(trans_sys, camera.transform_id);

    swap_draw_frame(&command_buffer, &render_pipeline->render_descripts, &render_pipeline->texture_descriptor, render_pipeline->model_render_data, render_pipeline->pipeline_layout, current_frame, heap_stack, cam->transform.position, render_pipeline->lights.light_amount);

    // ImGui::Render();
    // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer, nullptr);

    vkCmdEndRenderPass(command_buffer);
    vkEndCommandBuffer(command_buffer);

    VkSemaphore wait_semaphores[] = {render_pipeline->render_data.image_available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {render_pipeline->render_data.render_finished_semaphores[current_frame]};

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &render_pipeline->command_buffers[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    VkResult queue_result = vkQueueSubmit(render_pipeline->device.graphics_queue, 1, &submit_info, render_pipeline->render_data.in_flight_fences[current_frame]);

    if(queue_result != VK_SUCCESS)
        return queue_result;

    VkSwapchainKHR swap_chains[] = {render_pipeline->swap_chain.swap_chain};

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = 0; // Optional

    result = vkQueuePresentKHR(render_pipeline->device.present_queue, &present_info);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}
