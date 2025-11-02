#include "render_pipeline.h"


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
    

    instance = new Instance(application_name, enable_validation);

    assert(glfwCreateWindowSurface(instance->get_instance(), main_window, nullptr, &surface) == VK_SUCCESS);

    device = new Device(instance->get_instance(), surface, enable_validation);
    swap_chain = new SwapChain(main_window, device->get_physical_device(), surface, device->get_virtual_device());


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
}

RenderPipeline::~RenderPipeline()
{
    cleanup();
}

void RenderPipeline::cleanup()
{
    glfwDestroyWindow(main_window);
    delete swap_chain;
    vkDestroySurfaceKHR(instance->get_instance(), surface, nullptr);
    delete instance;
    delete device;
    glfwTerminate();
}

std::vector<char> RenderPipeline::load_shader(const std::string& file_name)
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