#pragma once
#include <vector>
#include <filesystem>
#include <string>
#include "../../../external/imgui_test/imgui.h"
#include "../../renderer/render_pipeline.h"
#include "../../engine/entity_manager/entity_manager.h"
#include "../../renderer/texture/vulkan/texture.h"

struct FileExplorer
{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    std::string current_directory;
};

void get_folders(const char* folder_to_look, std::vector<std::string>& result);

void get_folders(const std::string folder_to_look, std::vector<std::string>& result);

void get_folders(FileExplorer& file_explorer);

FileExplorer init_file_explorer();

void start_file_explorer(FileExplorer& file_explorer, RenderPipeline* render);

void end_file_explorer();
