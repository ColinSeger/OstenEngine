#include "render_pipeline.h"

void swap_draw_frame(VkCommandBuffer& command_buffer, Renderable& render_this, VkPipelineLayout pipeline_layout, Model& model, uint8_t frame)
{
    VkBuffer vertex_buffers[] = {model.vertex_buffer};
    VkDeviceSize offsets[] = {0};
    if(model.index_amount > 0){
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

        vkCmdBindIndexBuffer(command_buffer, model.index_buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &render_this.descriptor_sets[frame], 0, nullptr);

        vkCmdDrawIndexed(command_buffer, model.index_amount, 1, 0, 0, 0);
    }
}

RenderPipeline::RenderPipeline(const int width, const int height, const char* application_name)
{
    Debug::log((char*)"sHIT");
    glfwSetErrorCallback([](int code, const char* desc) {
        printf("GLFW ERROR %d: %s\n", code, desc);
    });
    if(!glfwInit()){
        puts("glfwInit failed");
        assert(false && "GLFW Failed to open");
    }


    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);

    #ifdef NDEBUG
        const std::vector<const char*> validation_layers = {};
    #else
        const std::vector<const char*> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
    #endif

    instance = Instance::create_instance(application_name, validation_layers);

    VkResult result = glfwCreateWindowSurface(instance, main_window, nullptr, &surface);

    if(result != VK_SUCCESS){
        assert(false && "Failed to create surface");
    }

    create_device(device, instance, surface, validation_layers);



    // ModelLoader::parse_obj("assets/debug_assets/napoleon.obj", vertices, indices);

    // ModelLoader::de_serialize("assets/debug_assets/napoleon.bin", vertices, indices);

    restart_swap_chain();

    ModelLoader::parse_obj(model_location, vertices, indices);
    models.emplace_back(ModelLoader::create_model(device, command_pool, vertices, indices));
    vertices.clear();
    indices.clear();

    ModelLoader::parse_obj("assets/debug_assets/napoleon.obj", vertices, indices);
    models.emplace_back(ModelLoader::create_model(device, command_pool, vertices, indices));
    vertices.clear();
    indices.clear();

    create_descriptor_set_layout(device.virtual_device, descriptor_set_layout);

    create_uniform_buffers();
    create_descriptor_pool(descriptor_pool, device.virtual_device);
    create_descriptor_sets(descriptor_pool, device.virtual_device, descriptor_set_layout, image_view, texture_sampler, to_render);


    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

    if(vkCreatePipelineLayout(device.virtual_device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS){
        assert(false && "Failed to create pipeline");
    }

    shader();

    create_sync_objects();
}

RenderPipeline::~RenderPipeline()
{
    cleanup();
}

void RenderPipeline::cleanup()
{
    vkDeviceWaitIdle(device.virtual_device);

    vkDestroyDescriptorPool(device.virtual_device, descriptor_pool, nullptr);

    vkDestroyDescriptorSetLayout(device.virtual_device, descriptor_set_layout, nullptr);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device.virtual_device, image_available_semaphores[i], nullptr);
        vkDestroySemaphore(device.virtual_device, render_finished_semaphores[i], nullptr);
        vkDestroyFence(device.virtual_device, in_flight_fences[i], nullptr);
    }
    glfwDestroyWindow(main_window);

    vkDestroyPipeline(device.virtual_device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device.virtual_device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device.virtual_device, render_pass, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);

    destroy_device(device);

    vkDestroyInstance(instance, nullptr);

    glfwTerminate();
}

void RenderPipeline::draw_frame()
{
    vkWaitForFences(device.virtual_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    static uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(device.virtual_device, swap_chain.swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        restart_swap_chain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    update_uniform_buffer(current_frame);

    vkResetFences(device.virtual_device, 1, &in_flight_fences[current_frame]);

    vkResetCommandBuffer(command_buffers[current_frame], 0);

    VkCommandBuffer command_buffer = command_buffers[current_frame];
    CommandBuffer::record_command_buffer(command_buffer);


    RenderPass::start_render_pass(command_buffer, swap_chain_images.swap_chain_framebuffers[image_index], render_pass, swap_chain.screen_extent);

    bind_pipeline(command_buffer, graphics_pipeline, swap_chain.screen_extent);

    // swap_draw_frame(command_buffer, to_render, pipeline_layout, render_buffer, static_cast<uint32_t>(indices.size()), current_frame);
    int test = 0;
    for(Renderable& render : to_render){
        test++;
        if(test <= 2){
            swap_draw_frame(command_buffer, render, pipeline_layout, models[0], current_frame);
        }else{
            swap_draw_frame(command_buffer, render, pipeline_layout, models[1], current_frame);
        }
    }

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer, nullptr);

    vkCmdEndRenderPass(command_buffer);

    RenderPass::end_render_pass(command_buffer);

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
        assert(false);
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

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        restart_swap_chain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RenderPipeline::update_uniform_buffer(uint8_t current_image) {

    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    // glm::vec3 forward = glm::fo
    Vector3 test = camera_location.position + Transformations::forward_vector(camera_location);
    Vector3 up = Transformations::up_vector(camera_location);
    glm::vec3 pos = {camera_location.position.x ,camera_location.position.x ,camera_location.position.x};

    glm::mat4 view = glm::lookAt(pos, {test.x, test.y, test.z}, {0, 0, 1});
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), swap_chain.screen_extent.width / (float) swap_chain.screen_extent.height, 0.1f, 2000.0f);
    proj[1][1] *= -1;
    // time = 1;
    for (size_t render_index = 0; render_index < to_render.size(); render_index++)
    {
        glm::mat4 inital_rotation = glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model = Transformations::get_model_matrix(to_render[render_index].transform) * inital_rotation;

        Matrix model2 {};

        UniformBufferObject ubo{
            ubo.model = model,
            ubo.view = view,
            ubo.proj = proj
        };

        memcpy(to_render[render_index].uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
    }
}

void RenderPipeline::create_uniform_buffers() {
    for (size_t render_index = 0; render_index < to_render.size(); render_index++)
    {
       VkDeviceSize bufferSize = sizeof(UniformBufferObject);
       Renderable& render_this = to_render[render_index];

        render_this.uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        render_this.uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
        render_this.uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            CommandBuffer::create_buffer(
                device,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                bufferSize,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                render_this.uniform_buffers[i],
                render_this.uniform_buffers_memory[i]
            );

            vkMapMemory(device.virtual_device, render_this.uniform_buffers_memory[i], 0, bufferSize, 0, &render_this.uniform_buffers_mapped[i]);
        }
    }
}

void RenderPipeline::create_uniform_buffer(Renderable& render_this) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    render_this.uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    render_this.uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
    render_this.uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CommandBuffer::create_buffer(
            device,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            bufferSize,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            render_this.uniform_buffers[i],
            render_this.uniform_buffers_memory[i]
        );

        vkMapMemory(device.virtual_device, render_this.uniform_buffers_memory[i], 0, bufferSize, 0, &render_this.uniform_buffers_mapped[i]);
    }
}

void RenderPipeline::restart_swap_chain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(main_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(main_window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device.virtual_device);

    if(swap_chain_images.swap_chain_images.size() > 0){
        vkDestroyImageView(device.virtual_device, image_view, nullptr);
        vkDestroySampler(device.virtual_device, texture_sampler, nullptr);
        clean_swap_chain(device.virtual_device, swap_chain, swap_chain_images);

        vkDestroyCommandPool(device.virtual_device, command_pool, nullptr);

        create_swap_chain(main_window, &device, surface, swap_chain);
        create_swap_chain_images(swap_chain, &device, surface, swap_chain_images);

    }else{
        create_swap_chain(main_window, &device, surface, swap_chain);
        create_swap_chain_images(swap_chain, &device, surface, swap_chain_images);
        create_render_pass();
    }
    command_pool = CommandBuffer::create_command_pool(device, surface);

    swap_chain_images.depth_image_view = create_depth_resources(&device, swap_chain.screen_extent, swap_chain_images.depth_image_memory, swap_chain_images.depth_image);

    create_frame_buffers(swap_chain_images, device.virtual_device, render_pass, swap_chain_images.depth_image_view, swap_chain.screen_extent);

    VkImage image_test = Texture::create_texture_image(device, texture_location, command_pool);

    image_view = Texture::create_image_view(device.virtual_device, image_test , VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    texture_sampler = Texture::create_texture_sampler(device);

    CommandBuffer::create_command_buffers(command_buffers, device.virtual_device, command_pool, MAX_FRAMES_IN_FLIGHT);
}

std::vector<char> load_shader(const std::string& file_name)
{
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if(!file.is_open()){
        assert(false && "Failed to load shaders");
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}

VkShaderModule create_shader(const std::vector<char>& code, VkDevice virtual_device) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(virtual_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void RenderPipeline::shader()
{
    //Move this later
    auto vertex_shader = load_shader("src/renderer/shaders/vert.spv");
    auto fragment_shader = load_shader("src/renderer/shaders/frag.spv");

    VkShaderModule vertex_module = create_shader(vertex_shader, device.virtual_device);
    VkShaderModule fragment_module = create_shader(fragment_shader, device.virtual_device);

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

    auto binding_description = get_binding_description();
    auto attribute_descriptions = get_attribute_descriptions();

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

    const VkDynamicState dynamic_states[] = {
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

    VkResult result = vkCreateGraphicsPipelines(device.virtual_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline);
    if(result != VK_SUCCESS){
        assert(false);
    }

    vkDestroyShaderModule(device.virtual_device, fragment_module, nullptr);
    vkDestroyShaderModule(device.virtual_device, vertex_module, nullptr);
}

void RenderPipeline::create_render_pass()
{
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription color_attachment{};
    color_attachment.format = swap_chain.swap_chain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = Texture::find_depth_formats(device.physical_device);
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkAttachmentDescription description_attachments[2] = {color_attachment, depth_attachment};

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = sizeof(description_attachments) / sizeof(description_attachments[0]);
    render_pass_info.pAttachments = description_attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if(vkCreateRenderPass(device.virtual_device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS){
        assert(false);
    }
}

void RenderPipeline::create_sync_objects()
{
    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if(vkCreateSemaphore(device.virtual_device, &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS){
            assert(false);
        }
        if(vkCreateSemaphore(device.virtual_device, &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS){
            assert(false);
        }
        if(vkCreateFence(device.virtual_device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS){
            assert(false);
        }
    }
}
