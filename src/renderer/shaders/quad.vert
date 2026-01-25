#version 450

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
} camera_buffer;

layout(set = 0, binding = 1) readonly buffer ModelBuffer {
    mat4 model_matrix[];
} model_buffer;

layout(location = 0) in vec3 in_position;

void main()
{
    gl_Position = camera_buffer.proj * camera_buffer.view * model_buffer.model_matrix[0] * vec4(in_position, 1.0);
    //gl_Position = vec4(in_position.xy, 0.5, 1.0);

    // gl_Position = vec4(
    //        (gl_VertexIndex == 0) ? -1.0 : 3.0,
    //        (gl_VertexIndex == 2) ? -1.0 : 3.0,
    //        0.5,
    //        1.0
    //    );
}
