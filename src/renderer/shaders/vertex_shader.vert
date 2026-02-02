#version 460

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
} camera_buffer;

layout(set = 0, binding = 1) readonly buffer ModelBuffer {
    mat4 model_matrix[];
} model_buffer;

layout(set = 0, binding = 2) uniform LightBuffer {
    mat4 light_view_proj;
} light_buffer;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_cord;

layout(location = 0) out vec3 frag_normal;
layout(location = 1) out vec2 frag_tex_cord;
layout(location = 2) out vec4 frag_pos_light_space;

void main() {
    mat4 model = model_buffer.model_matrix[gl_InstanceIndex];
    vec4 world_pos = model * vec4(in_position, 1.0);

    gl_Position = camera_buffer.proj * camera_buffer.view * world_pos;

    frag_normal = normalize(mat3(model) * in_normal);
    frag_tex_cord = in_tex_cord;
    frag_pos_light_space = light_buffer.light_view_proj * world_pos;
}
