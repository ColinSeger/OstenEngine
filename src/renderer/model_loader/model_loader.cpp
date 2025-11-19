#include "model_loader.h"


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
}/**/


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
    uint32_t file_size = file_stream.tellg();
    file_stream.seekg(0);
    char* file = new char[file_size];
    //memset(file, 0, file_size);

    file_stream.read(file, file_size);
    file_stream.close();

    std::vector<Vertex> vertex;

    OBJ_Mode current_mode = OBJ_Mode::None;
    std::string values[3];
    uint8_t index = -1;

    for (size_t i = 0; i < file_size; i++)
    {
        if(file[i] == '#') continue;
        if(current_mode == OBJ_Mode::None){
           if (file[i] == 'v') {
                i++;
                current_mode = OBJ_Mode::Vertex;
            } 
        }
        if(current_mode == OBJ_Mode::None) continue;
        
        if(file[i] == '\n'){
            if(current_mode == OBJ_Mode::Vertex){
                Vertex new_vertex;
                float x = std::stof(values[0]);
                float y = std::stof(values[1]);
                float z = std::stof(values[2]);
                new_vertex.position = glm::vec3(x, y, z);
                vertices.push_back(new_vertex);
                indices.push_back(vertices.size());
                // vertex.push_back(new_vertex);
                index = -1;
                values[0].clear();
                values[1].clear();
                values[2].clear();
                
                current_mode = OBJ_Mode::None;
            }
        }else{
            if(file[i] == ' ')
            {
                index++;
            }else{
                values[index].push_back(file[i]);
            }
        }
    }
    


    
}