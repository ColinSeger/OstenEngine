#pragma once
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include "../render_data/vulkan/render_data.h"

enum class ObjMode : uint8_t
{
    Vertex,
    Normal,
    TextureCord,
    Face,
    None,
    Comment
};
namespace ModelLoader
{
    const char valid_chars[14] = "0123456789.-/";

    bool is_valid_char(char c);

    void parse_obj(const char* path_of_obj, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    void serialize(const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    void de_serialize(const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
}/**/

