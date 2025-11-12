#include "model_loader.h"


std::vector<Vertex> model_loader::load_model(const char* model_path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    assert(tinyobj::LoadObj(&attrib, &shapes, &materials, &err, model_path) == true);
}