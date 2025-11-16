#include "render_pipeline.h"


void create_descriptor_pool(VkDescriptorPool& result, VkDevice virtual_device)
{
    VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_SAMPLER,                1000},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000},
};
    // VkDescriptorPool result;
    // VkDescriptorPoolSize pool_size{};
    // pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // pool_size.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.poolSizeCount = (uint32_t) IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);

    assert(vkCreateDescriptorPool(virtual_device, &pool_info, nullptr, &result) == VK_SUCCESS);
    // return result;
}

RenderPipeline::RenderPipeline(const int width, const int height, const char* application_name)
{
    assert(glfwInit() == GLFW_TRUE && "GLFW Failed to open");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    main_window = glfwCreateWindow(width, height, application_name, nullptr, nullptr);


    #ifdef NDEBUG
        const bool enable_validation = false;
    #else
        const bool enable_validation = true;
    #endif

    instance = Instance::create_instance(application_name, enable_validation);

    assert(glfwCreateWindowSurface(instance, main_window, nullptr, &surface) == VK_SUCCESS);

    device = new Device(instance, surface, enable_validation);

    model_loader::load_model("C:/Users/colin/Documents/Project/OstenEngine/GameEngine/assets/debug_assets/viking.obj", vertices, indices);

    

    restart_swap_chain();

    if(indices.size() > 0){
        CommandBuffer::create_vertex_buffer(device, vertices, vertex_buffer, vertex_buffer_memory, swap_chain->get_command_pool());
        CommandBuffer::create_index_buffer(device, indices, index_buffer, index_buffer_memory, swap_chain->get_command_pool());
    }

    create_descriptor_set_layout(device->get_virtual_device(), descriptor_set_layout);

    create_uniform_buffers();
    create_descriptor_pool(descriptor_pool, device->get_virtual_device());
    create_descriptor_sets(descriptor_sets, descriptor_pool, device->get_virtual_device(), descriptor_set_layout);


    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0; // Optional
    pipeline_layout_info.pPushConstantRanges = nullptr; // Optional

    assert(vkCreatePipelineLayout(device->get_virtual_device(), &pipeline_layout_info, nullptr, &pipeline_layout) == VK_SUCCESS && "Failed to create pipeline");
    
    shader();

    create_sync_objects();
}

RenderPipeline::~RenderPipeline()
{
    cleanup();
}

void RenderPipeline::cleanup()
{
    vkDeviceWaitIdle(device->get_virtual_device());
    delete swap_chain;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device->get_virtual_device(), uniform_buffers[i], nullptr);
        vkFreeMemory(device->get_virtual_device(), uniform_buffers_memory[i], nullptr);
    }
    vkDestroyDescriptorPool(device->get_virtual_device(), descriptor_pool, nullptr);

    vkDestroyDescriptorSetLayout(device->get_virtual_device(), descriptor_set_layout, nullptr);

    vkDestroyBuffer(device->get_virtual_device(), vertex_buffer, nullptr);
    vkFreeMemory(device->get_virtual_device(), vertex_buffer_memory, nullptr);
    vkDestroyBuffer(device->get_virtual_device(), index_buffer, nullptr);
    vkFreeMemory(device->get_virtual_device(), index_buffer_memory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device->get_virtual_device(), image_available_semaphores[i], nullptr);
        vkDestroySemaphore(device->get_virtual_device(), render_finished_semaphores[i], nullptr);
        vkDestroyFence(device->get_virtual_device(), in_flight_fences[i], nullptr);
    }
    glfwDestroyWindow(main_window);
    
    vkDestroyPipeline(device->get_virtual_device(), graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device->get_virtual_device(), pipeline_layout, nullptr);
    vkDestroyRenderPass(device->get_virtual_device(), render_pass, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    
    delete device;
    vkDestroyInstance(instance, nullptr);
    
    glfwTerminate();
}

void RenderPipeline::draw_model(Renderable to_draw)
{
    
    model_loader::load_model(to_draw.mesh_location, vertices, indices);
    
    if(indices.size() > 0){
        CommandBuffer::create_vertex_buffer(device, vertices, vertex_buffer, vertex_buffer_memory, swap_chain->get_command_pool());
        CommandBuffer::create_index_buffer(device, indices, index_buffer, index_buffer_memory, swap_chain->get_command_pool());
    }
/**/
    VkImage image_test = Texture::create_texture_image(device, to_draw.texture_location, swap_chain->get_command_pool());
    
    image_view = Texture::create_image_view(device->get_virtual_device(), image_test , VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    texture_sampler = Texture::create_texture_sampler(device);
    create_descriptor_sets(descriptor_sets, descriptor_pool, device->get_virtual_device(), descriptor_set_layout);
}

void RenderPipeline::draw_frame()
{
    vkWaitForFences(device->get_virtual_device(), 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
    
    static uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(device->get_virtual_device(), swap_chain->get_swap_chain(), UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        restart_swap_chain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    update_uniform_buffer(current_frame);
    
    vkResetFences(device->get_virtual_device(), 1, &in_flight_fences[current_frame]);

    vkResetCommandBuffer(command_buffers[current_frame], 0);

    VkCommandBuffer command_buffer = command_buffers[current_frame];
    CommandBuffer::record_command_buffer(command_buffer);

    swap_chain->start_render_pass(command_buffer ,image_index, render_pass);

    RenderBuffer render_buffer = {
        vertex_buffer,
        index_buffer
    };

    swap_chain->bind_pipeline(command_buffer, graphics_pipeline, pipeline_layout, descriptor_sets, render_buffer, static_cast<uint32_t>(vertices.size()), static_cast<uint32_t>(indices.size()), current_frame);

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

    VkResult queue_result = vkQueueSubmit(device->get_graphics_queue(), 1, &submit_info, in_flight_fences[current_frame]);

    assert(queue_result == VK_SUCCESS);

    VkSwapchainKHR swap_chains[] = {swap_chain->get_swap_chain()};

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr; // Optional

    
    result = vkQueuePresentKHR(device->get_present_queue(), &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        restart_swap_chain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RenderPipeline::update_uniform_buffer(uint8_t current_image) {
    float direction = 90.f;

    if(spin_direction){
        direction = -90;
    }

    static auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    // time = 1;
    UniformBufferObject ubo{};
    // ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(direction), glm::vec3(-1.0f, 0.0f, 0.0f));
    ubo.model = glm::rotate(glm::mat4(scale), time * glm::radians(direction), glm::vec3(spin_x, spin_y, spin_z));
    // ubo.model = glm::translate(glm::mat4(scale), glm::vec3(spin_x, spin_y, spin_z));

    ubo.view = glm::lookAt(glm::vec3(camera_thing[0], camera_thing[1], camera_thing[2]), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swap_chain->get_extent().width / (float) swap_chain->get_extent().height, 0.1f, 2000.0f);

    ubo.proj[1][1] *= -1;

    memcpy(uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

void RenderPipeline::create_uniform_buffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CommandBuffer::create_buffer(
            device,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            bufferSize,  
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            uniform_buffers[i], 
            uniform_buffers_memory[i]
        );

        vkMapMemory(device->get_virtual_device(), uniform_buffers_memory[i], 0, bufferSize, 0, &uniform_buffers_mapped[i]);
    }
}

void RenderPipeline::create_descriptor_sets(std::vector<VkDescriptorSet>& result, VkDescriptorPool& descriptor_pool, VkDevice virtual_device, VkDescriptorSetLayout& descriptor_set_layout)
{
    result.resize(MAX_FRAMES_IN_FLIGHT);

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor_set_layout);//Swap for normal array later?
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    assert(vkAllocateDescriptorSets(virtual_device, &allocInfo, result.data()) == VK_SUCCESS);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = image_view;
        image_info.sampler = texture_sampler;

        const uint8_t descriptor_size = 2;
        VkWriteDescriptorSet descriptor_writes[descriptor_size]{};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;
        descriptor_writes[0].pImageInfo = nullptr; // Optional
        descriptor_writes[0].pTexelBufferView = nullptr; // Optional

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        vkUpdateDescriptorSets(virtual_device, descriptor_size, descriptor_writes, 0, nullptr);
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
    vkDeviceWaitIdle(device->get_virtual_device());
    auto swap_chain_support = Setup::find_swap_chain_support(device->get_physical_device(), surface);
        VkExtent2D screen_extent = Setup::select_swap_chain_extent(swap_chain_support.surface_capabilities, main_window);
    if(swap_chain){
        delete swap_chain;
        
        swap_chain = new SwapChain(device->get_physical_device(), surface, device->get_virtual_device(), screen_extent);

    }else{
        swap_chain = new SwapChain(device->get_physical_device(), surface, device->get_virtual_device(), screen_extent);
        create_render_pass();
    }
    
    swap_chain->create_command_pool(device->get_physical_device());

    create_depth_resources();
    swap_chain->create_frame_buffers(render_pass, depth_image_view);

    //if(indices.size() > 0){
    //    CommandBuffer::create_vertex_buffer(device, vertices, vertex_buffer, vertex_buffer_memory, swap_chain->get_command_pool());
    //    CommandBuffer::create_index_buffer(device, indices, index_buffer, index_buffer_memory, swap_chain->get_command_pool());
    //}

    VkImage image_test = Texture::create_texture_image(device, "C:/Users/colin/Documents/Project/OstenEngine/GameEngine/assets/debug_assets/viking_room.png", swap_chain->get_command_pool());

    image_view = Texture::create_image_view(device->get_virtual_device(),image_test , VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    texture_sampler = Texture::create_texture_sampler(device);
/**/
    //VkImage image_test = Texture::create_texture_image(device, "C:/Users/colin/Documents/Project/OstenEngine/GameEngine/assets/debug_assets/viking_room.png", swap_chain->get_command_pool());
    
    //image_view = Texture::create_image_view(device->get_virtual_device(),image_test , VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    //texture_sampler = Texture::create_texture_sampler(device);

    CommandBuffer::create_command_buffers(command_buffers, device->get_virtual_device(), swap_chain->get_command_pool(), MAX_FRAMES_IN_FLIGHT);
}

std::vector<char> load_shader(const std::string& file_name)
{
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    assert(file.is_open() == true && "Failed to load shaders");

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
}

VkShaderModule RenderPipeline::create_shader(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device->get_virtual_device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void RenderPipeline::shader()
{
    //Move this later
    auto vertex_shader = load_shader("src/renderer/shaders/vert.spv");
    auto fragment_shader = load_shader("src/renderer/shaders/frag.spv");

    VkShaderModule vertex_module = create_shader(vertex_shader);
    VkShaderModule fragment_module = create_shader(fragment_shader);

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

    auto binding_description = Vertex::get_binding_description();
    auto attribute_descriptions = Vertex::get_attribute_descriptions();

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
    viewport.width = (float) swap_chain->get_extent().width;
    viewport.height = (float) swap_chain->get_extent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain->get_extent();

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

    VkResult result = vkCreateGraphicsPipelines(device->get_virtual_device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline);
    assert(result == VK_SUCCESS);

    vkDestroyShaderModule(device->get_virtual_device(), fragment_module, nullptr);
    vkDestroyShaderModule(device->get_virtual_device(), vertex_module, nullptr);
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
    color_attachment.format = swap_chain->get_image_format();
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
    depth_attachment.format = Texture::find_depth_formats(device->get_physical_device());
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

    assert(vkCreateRenderPass(device->get_virtual_device(), &render_pass_info, nullptr, &render_pass) == VK_SUCCESS); 
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
        assert(vkCreateSemaphore(device->get_virtual_device(), &semaphore_info, nullptr, &image_available_semaphores[i]) == VK_SUCCESS);
        assert(vkCreateSemaphore(device->get_virtual_device(), &semaphore_info, nullptr, &render_finished_semaphores[i]) == VK_SUCCESS);
        assert(vkCreateFence(device->get_virtual_device(), &fence_info, nullptr, &in_flight_fences[i]) == VK_SUCCESS);
    }
}

void RenderPipeline::create_depth_resources()
{
    VkImage depth_image;
    VkFormat depth_formating = Texture::find_depth_formats(device->get_physical_device());//TODO make a thing that searches for format
    ImageSize image_size{
        swap_chain->get_extent().width,
        swap_chain->get_extent().height
    };
    Texture::create_image(device, image_size, depth_formating, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);

    depth_image_view = Texture::create_image_view(device->get_virtual_device() ,depth_image, depth_formating, VK_IMAGE_ASPECT_DEPTH_BIT);
}
