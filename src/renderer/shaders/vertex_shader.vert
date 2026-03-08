#version 460

#define MAX_LIGHTS 8

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view_projection;
} camera_buffer;

layout(set = 0, binding = 1) uniform LightBuffer {
    mat4 light_views[MAX_LIGHTS];
} light_buffer;

layout(set = 1, binding = 0) readonly buffer ModelBuffer {
    mat4 model_matrix[];
} model_buffer;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_cord;

layout(location = 0) out vec3 frag_position;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec2 frag_tex_cord;
layout(location = 3) out vec4 frag_pos_light_space[MAX_LIGHTS];

void main() {
    mat4 model = model_buffer.model_matrix[gl_InstanceIndex];
    vec4 world_pos = model * vec4(in_position, 1.0);

    gl_Position = camera_buffer.view_projection * world_pos;

	frag_normal = normalize(transpose(inverse(mat3(model))) * in_normal);
    //frag_normal = normalize(mat3(model) * in_normal);
    frag_tex_cord = in_tex_cord;

    for (int i = 0; i < MAX_LIGHTS; i++){
        frag_pos_light_space[i] = light_buffer.light_views[i] * world_pos;
    }
    frag_position = world_pos.xyz;
}
