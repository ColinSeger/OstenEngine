#include "model_loader.h"

/*
void model_loader::load_model(const char* model_path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string warning;

    if(tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &err, model_path) != true) {
        //char *buf;
        //char* file = getcwd(buf, 1024);

        auto test = std::filesystem::current_path();
        auto bro = test.filename().string();
        throw std::runtime_error(bro);
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texture_cord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
    }
}*/

inline OBJ_Mode select_mode(char* char_to_check)//This is ass
{
    if(*char_to_check == '#'){
        return OBJ_Mode::Comment;
    }
    if (*char_to_check == 'v') {
        char_to_check++;
        if(*char_to_check == ' '){
            return OBJ_Mode::Vertex;
        }
        if(*char_to_check == 't'){
            return OBJ_Mode::TextureCord;
        }
    }
    if(*char_to_check == 'f'){
        return OBJ_Mode::Face;
    }
    return OBJ_Mode::None;
}


void model_loader::parse_obj(const char* path_of_obj, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<std::string> logs)
{
    std::ifstream file_stream;

    //std::ios_base::in | std::ios_base::ate | std::ios_base::binary

    file_stream.open(path_of_obj, std::ios_base::in | std::ios_base::ate);

    if(!file_stream.is_open())
    {
        vertices.emplace_back(Vertex{});
        indices.emplace_back(0);

        auto file_path = std::filesystem::current_path();
        auto filepath_str = file_path.filename().string();
        std::string error = filepath_str;
        error.push_back(errno);

        throw std::runtime_error(error);
        //TODO LOG failed model load
        return;
    }
    size_t file_size = file_stream.tellg();
    file_stream.seekg(0);
    char* file = new char[file_size];
    //memset(file, 0, file_size);

    file_stream.read(file, file_size);
    file_stream.close();

    std::vector<Vertex> vertex;

    std::vector<uint32_t> texture_index;
    std::vector<glm::vec2> texture_cord;

    OBJ_Mode current_mode = OBJ_Mode::None;
    std::string values[4];
    uint8_t index = 0;
    std::string command;
    Vertex new_vertex;
    uint32_t vertex_id = 0;

    vertex.reserve(file_size/40);
    indices.reserve(file_size/60);
    texture_cord.reserve(file_size/60);
    texture_index.reserve(file_size/60);

    for (size_t i = 0; i < file_size; i++)
    {
        char value = file[i];

        if(value == '\n' || current_mode == OBJ_Mode::None){

            switch (current_mode)
            {
            case OBJ_Mode::None:
                current_mode = select_mode(&file[i]);
                if(current_mode == OBJ_Mode::TextureCord) i++;
                index = 0;
            break;
            case OBJ_Mode::Comment:
                current_mode = OBJ_Mode::None;
            break;
            case OBJ_Mode::Vertex:
                
                new_vertex.position = glm::vec3(std::stof(values[0]), std::stof(values[1]), std::stof(values[2]));
                vertex.push_back(new_vertex);
                
                current_mode = OBJ_Mode::None;
            break;
            case OBJ_Mode::Face:
                for (std::string& index : values)
                {
                    if(index.length() <= 0) continue;
                    indices.push_back(std::stoi(index) -1);
                    for (size_t i = 0; i < index.size(); i++)
                    {
                        if(index[i] == '/')
                        {
                            texture_index.push_back(std::stoi(&index[i+1]) -1);
                            break;
                        }
                    }
                }
                current_mode = OBJ_Mode::None;
            break;
            case OBJ_Mode::TextureCord:
                texture_cord.emplace_back(std::stof(values[0]), 1.f - std::stof(values[1]));
                current_mode = OBJ_Mode::None;
            break;
            case OBJ_Mode::Normal:
                //Todo
            break;
            default:
                break;
            }
            for (std::string& index : values)
            {
                index.clear();
            }
            continue;
        }

        if(current_mode == OBJ_Mode::None || current_mode == OBJ_Mode::Comment) continue;

        if(value == ' ')
        {
            if(values[0].length() > 0){
                index++;                    
            }

        }else{
            if(index >= 4) continue;
            values[index].push_back(value);
        }
    }
    for (size_t i = 0; i < indices.size(); i++)
    {
        vertex[indices[i]].texture_cord = texture_cord[texture_index[i]];
    }
    
    
    vertices = vertex;

    delete[] file;
    
}
