#include "model_loader.h"


Model ModelLoader::load_model(Device& device, VkCommandPool command_pool, std::string filename)
{
    Model model;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    char extention[3];
    extention[0] = filename[filename.length() -2];
    extention[1] = filename[filename.length() -1];
    extention[2] = filename[filename.length() -0];

    if(extention == "obj"){
        parse_obj(filename.c_str(), vertices, indices);
    }else if(extention == "bin"){
        de_serialize(filename.c_str(), vertices, indices);
    }

    model = create_model(device, command_pool, vertices, indices);

    return model;
}

bool ModelLoader::is_valid_char(char c)
{
    for(char valid : valid_chars)
    {
        if(c == valid) return true;
    }
    return false;
}

static inline ObjMode select_mode(char* char_to_check)//This is ass
{
    if(*char_to_check == '#'){
        return ObjMode::Comment;
    }
    if (*char_to_check == 'v') {
        char_to_check++;
        if(*char_to_check == ' '){
            return ObjMode::Vertex;
        }
        if(*char_to_check == 't'){
            return ObjMode::TextureCord;
        }
    }
    if(*char_to_check == 'f'){
        return ObjMode::Face;
    }
    return ObjMode::None;
}

void ModelLoader::parse_obj(const char* path_of_obj, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    std::ifstream file_stream;

    file_stream.open(path_of_obj, std::ios_base::in | std::ios_base::ate);

    size_t index_offset = indices.size();

    if(!file_stream.is_open())
    {
        vertices.emplace_back(Vertex{});
        indices.emplace_back(0);

        auto file_path = std::filesystem::current_path();
        auto filepath_str = file_path.filename().string();
        std::string error = filepath_str;
        error.push_back(errno);

        // throw std::runtime_error(error);
        Debug::log((char*)"Failed to load model");
        //TODO LOG failed model load
        return;
    }

    size_t file_size = file_stream.tellg();
    file_stream.seekg(0);
    char* file = (char*)malloc(sizeof(char) * file_size);

    file_stream.read(file, file_size);
    file_stream.close();

    std::vector<Vertex> vertex;

    std::vector<uint32_t> texture_index;
    std::vector<Vector2> texture_cord;

    ObjMode current_mode = ObjMode::None;
    std::string values[3];

    uint8_t char_index = 0;
    Vertex new_vertex {};

    vertex.reserve(file_size/40);
    indices.reserve(file_size/60);
    texture_cord.reserve(file_size/60);
    texture_index.reserve(file_size/60);

    for (size_t i = 0; i < file_size; i++)
    {
        char value = file[i];

        if(value == '\n' || current_mode == ObjMode::None){

            switch (current_mode)
            {
            case ObjMode::None:
                current_mode = select_mode(&file[i]);
                if(current_mode == ObjMode::TextureCord) i++;//do i even need to?
                char_index = 0;
                continue;
            break;
            case ObjMode::Comment:
                
            break;
            case ObjMode::Vertex:
                
                new_vertex.position = {std::stof(values[0]), std::stof(values[1]), std::stof(values[2])};
                vertex.emplace_back(new_vertex);
            break;
            case ObjMode::Face:
                for (std::string& index : values)
                {
                    if(index.length() <= 0) continue;
                    indices.push_back(std::stoi(index) -1 + index_offset);
                    for (size_t i = 0; i < index.size(); i++)
                    {
                        if(index[i] == '/')
                        {
                            texture_index.push_back(std::stoi(&index[i+1]) -1);
                            break;
                        }
                    }
                }
            break;
            case ObjMode::TextureCord:
                texture_cord.emplace_back(Vector2{std::stof(values[0]), 1.f - std::stof(values[1])});
            break;
            case ObjMode::Normal:
                //Todo
            break;
            default:
                break;
            }
            for (std::string& index : values)
            {
                index.clear();
            }
            current_mode = ObjMode::None;
            continue;
        }

        if(current_mode == ObjMode::None || current_mode == ObjMode::Comment) continue; // do I even need this?

        if(value == ' ' && values[0].length() > 0){
            char_index++;                    
            continue;
        }
        else if (is_valid_char(value)){
            values[char_index].push_back(value);
        }
    }
    for (size_t i = 0; i < indices.size() - index_offset; i++)
    {
        vertex[indices[i]].texture_cord = texture_cord[texture_index[i]];
    }
    for (size_t i = 0; i < vertex.size(); i++)//Temp solution
    {
        vertices.emplace_back(vertex[i]);
    }
    
    free(file);
    
}

void ModelLoader::serialize(const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    std::ofstream file(filename, std::ios::binary);

    if(!file.is_open()){
        abort();
    }
    uint32_t index_start = vertices.size() * sizeof(Vertex);

    file.write(reinterpret_cast<const char*>(&index_start), sizeof(uint32_t));

    for(Vertex vertex : vertices){
        file.write(reinterpret_cast<const char*>(&vertex), sizeof(Vertex));
    }

    for(uint32_t index : indices){
        file.write(reinterpret_cast<const char*>(&index), sizeof(uint32_t));
    }

    file.close();
}

void ModelLoader::de_serialize(const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    std::ifstream file(filename, std::ios::binary);

    if(!file.is_open()){
        abort();//
    }
    file.seekg (0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0);
    
    size_t index_start = 0;//represents where vertexes end and indices start

    file.read(reinterpret_cast<char*>(&index_start), sizeof(uint32_t));
    size_t vertex_done = index_start;

    //Allocate chunk of memory for vertices and indexes
    Vertex* vertex_ptr = (Vertex*)calloc(vertex_done / sizeof(Vertex), sizeof(Vertex));
    uint32_t* index_ptr = (uint32_t*)malloc(file_size - (index_start +1));

    Vertex* read_this = vertex_ptr;
    uint32_t* read_index = index_ptr;

    file.read(reinterpret_cast<char*>(vertex_ptr), vertex_done);
    file.read(reinterpret_cast<char*>(index_ptr), file_size - vertex_done);
    file.close();

    for(size_t index = 0; index < vertex_done; index += sizeof(Vertex)){
        vertices.emplace_back(*read_this);
        read_this++;
    }
    
    for(size_t index = vertex_done; index <= file_size; index += sizeof(uint32_t)){
        indices.emplace_back(*read_index);
        read_index++;
    }
    
    free(vertex_ptr);
    free(index_ptr);
}

Model ModelLoader::create_model(Device& device, VkCommandPool command_pool, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    Model result {};
    result.index_amount = indices.size();
    CommandBuffer::create_vertex_buffer(device, vertices, result.vertex_buffer, result.vertex_buffer_memory, command_pool);
    CommandBuffer::create_index_buffer(device, indices, result.index_buffer, result.index_buffer_memory, command_pool);


    return result;
}