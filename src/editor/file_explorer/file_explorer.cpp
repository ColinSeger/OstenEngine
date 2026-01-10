//#include "file_explorer.h"
#pragma once
#include <filesystem>
#include <vector>
#include <string>
#include "../../../external/imgui_test/imgui.h"
#include "../../renderer/render_pipeline.cpp"
#include "../../engine/message_system/message.cpp"

struct FileExplorer{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    std::string current_directory;
};

void get_folders(const char* folder_to_look, std::vector<std::string>& result){
    for(auto& folders : std::filesystem::directory_iterator(folder_to_look)){
        if (folders.is_directory()){
            result.push_back(folders.path().string());
        }
    }
}

void get_folders(const std::string folder_to_look, std::vector<std::string>& result){
    for(auto& folders : std::filesystem::directory_iterator(folder_to_look)){
        if (folders.is_directory()){
            result.push_back(folders.path().string());
        }
    }
}

void get_folders(FileExplorer& file_explorer){
    for(auto& folders : std::filesystem::directory_iterator(file_explorer.current_directory)){
        if (folders.is_directory()){
            file_explorer.folders.push_back(folders.path().string());
        }
        if(folders.is_regular_file()){
            file_explorer.files.push_back(folders.path().string());
        }
    }
}

FileExplorer init_file_explorer(){
    FileExplorer result;
    auto file_path = std::filesystem::current_path();

    result.current_directory = file_path.string();

    get_folders(result.current_directory.c_str(), result.folders);

    return result;
}

void start_file_explorer(FileExplorer& file_explorer, RenderPipeline* render_pipeline){ //TODO Optimize as it's very slow
    ImGui::Begin("FolderView");

    std::string current_dir = file_explorer.current_directory;
    if(ImGui::Button("Go Back")){
        auto file_path = std::filesystem::current_path();
        if(file_path.string() != file_explorer.current_directory){
            file_explorer.folders.clear();
            file_explorer.files.clear();
            file_explorer.current_directory = file_path.string();
        }
    }

    for (size_t i = 0; i < file_explorer.folders.size(); i++){
        ImGui::PushID(i);
        if(ImGui::Button(file_explorer.folders[i].c_str()))
        {
            // ImGui::Text(file_explorer.folders[i].c_str());
            file_explorer.current_directory = file_explorer.folders[i];
        }
        ImGui::Spacing();

        ImGui::PopID();
    }

    for (size_t i = 0; i < file_explorer.files.size(); i++){
        ImGui::PushID(i);
        if(ImGui::Button(file_explorer.files[i].c_str())){
            Message load{
                static_cast<uint32_t>(file_explorer.files[i].size()),
                MessageType::LoadModel,
                (void*)file_explorer.files[i].c_str()
            };

            add_message(load);
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
