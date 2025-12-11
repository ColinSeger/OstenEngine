//#include "file_explorer.h"
#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include "../../../external/imgui_test/imgui.h"
// #include "../../engine/entity_manager/entity_manager.h"
// #include "../../renderer/texture/vulkan/texture.cpp"

struct FileExplorer
{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    std::string current_directory;
};

// static void create_entity(RenderPipeline* render_pipeline){
//     uint16_t id = add_transform();
//     Renderable first_obj;
//     first_obj.transform_index = id;
//     render_pipeline->create_uniform_buffer(first_obj);
//     VkImage image_test;
//     if(render_pipeline->to_render.size() < 2){
//         image_test = Texture::create_texture_image(render_pipeline->device, "assets/debug_assets/viking_room.png", render_pipeline->command_pool);
//     }else{
//         image_test = Texture::create_texture_image(render_pipeline->device, "assets/debug_assets/napoleon_texture.png", render_pipeline->command_pool);
//     }
//     TextureImage texture {};
//     texture.image_view = Texture::create_image_view(render_pipeline->device.virtual_device, image_test , VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
//     texture.texture_sampler = Texture::create_texture_sampler(render_pipeline->device);

//     create_descriptor_set(render_pipeline->device.virtual_device, first_obj, render_pipeline->descriptor_pool, render_pipeline->descriptor_set_layout, texture.image_view, texture.texture_sampler);
//     render_pipeline->to_render.push_back(first_obj);
//     // vkDestroyImageView(render_pipeline->device.virtual_device, image_view, nullptr);
//     // vkDestroySampler(render_pipeline->device.virtual_device, texture_sampler, nullptr);
//     Entity entity{};
//     entity.components.push_back({id, 1});
//     EntityManager::add_entity(entity, "Test");
// }

void get_folders(const char* folder_to_look, std::vector<std::string>& result)
{
    for(auto& folders : std::filesystem::directory_iterator(folder_to_look)){
        if (folders.is_directory()){
            result.push_back(folders.path().string());
        }
    }
}

void get_folders(const std::string folder_to_look, std::vector<std::string>& result)
{
    for(auto& folders : std::filesystem::directory_iterator(folder_to_look)){
        if (folders.is_directory()){
            result.push_back(folders.path().string());
        }
    }
}

void get_folders(FileExplorer& file_explorer)
{
    for(auto& folders : std::filesystem::directory_iterator(file_explorer.current_directory)){
        if (folders.is_directory()){
            file_explorer.folders.push_back(folders.path().string());
        }
        if(folders.is_regular_file()){
            file_explorer.files.push_back(folders.path().string());
        }
    }
}

FileExplorer init_file_explorer()
{
    FileExplorer result;
    auto file_path = std::filesystem::current_path();

    result.current_directory = file_path.string();

    get_folders(result.current_directory.c_str(), result.folders);

    return result;
}

void start_file_explorer(FileExplorer& file_explorer)//TODO Optimize as it's very slow
{

    ImGui::Begin("FolderView");

    std::string current_dir = file_explorer.current_directory;

    for (size_t i = 0; i < file_explorer.folders.size(); i++)
    {
        ImGui::PushID(i);
        if(ImGui::Button(file_explorer.folders[i].c_str()))
        {
            // ImGui::Text(file_explorer.folders[i].c_str());
            file_explorer.current_directory = file_explorer.folders[i];
        }
        ImGui::Spacing();

        ImGui::PopID();
    }

    for (size_t i = 0; i < file_explorer.files.size(); i++)
    {
        ImGui::PushID(i);
        ImGui::Text(file_explorer.files[i].c_str());
        if(ImGui::Button(file_explorer.files[i].c_str())){
            // ModelLoader::parse_obj(file_explorer.files[i].c_str(), render_pipeline->vertices, render_pipeline->indices);
            // ModelLoader::de_serialize(file_explorer.files[i].c_str(), render_pipeline->vertices, render_pipeline->indices);
            // render_pipeline->models.emplace_back(ModelLoader::create_model(render_pipeline->device, render_pipeline->command_pool, render_pipeline->vertices, render_pipeline->indices));

            // create_entity(render_pipeline);
        }

        ImGui::Spacing();

        ImGui::PopID();
    }

    if(current_dir != file_explorer.current_directory){
        file_explorer.folders.clear();

        get_folders(file_explorer);
    }
}

void end_file_explorer()
{
    ImGui::End();
}
