#include "file_explorer.h"


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