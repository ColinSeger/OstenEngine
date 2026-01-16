#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_tex_cord;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_tex_cord;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.08;

const vec3 COLOR = vec3(1,1,1);

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 1.0);

    vec3 normal_world_space = normalize(mat3(ubo.model) * in_color);

    float light_intensity = AMBIENT + max(dot(normal_world_space, DIRECTION_TO_LIGHT), 0);

    frag_color = COLOR * light_intensity;

    frag_tex_cord = in_tex_cord;
}
