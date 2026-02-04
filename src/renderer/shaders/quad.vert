#version 450

layout(set = 0, binding = 0) uniform LightBuffer {
    mat4 view;
    mat4 proj;
} light_buffer;

layout(set = 1, binding = 0) readonly buffer ModelBuffer {
    mat4 model_matrix[];
} model_buffer;

layout(location = 0) in vec3 in_position;

void main() {
    gl_Position = light_buffer.proj * light_buffer.view * model_buffer.model_matrix[gl_InstanceIndex] * vec4(in_position, 1.0);
}
