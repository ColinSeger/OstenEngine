#include "render_data.h"

VkVertexInputBindingDescription get_binding_description() {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_description;
}

VertexAtributes get_attribute_descriptions() {
    VertexAtributes attribute_descriptions{};
    attribute_descriptions.array[0].binding = 0;
    attribute_descriptions.array[0].location = 0;
    attribute_descriptions.array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions.array[0].offset = offsetof(Vertex, position);

    attribute_descriptions.array[1].binding = 0;
    attribute_descriptions.array[1].location = 1;
    attribute_descriptions.array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions.array[1].offset = offsetof(Vertex, color);

    attribute_descriptions.array[2].binding = 0;
    attribute_descriptions.array[2].location = 2;
    attribute_descriptions.array[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions.array[2].offset = offsetof(Vertex, texture_cord);

    return attribute_descriptions;
}
