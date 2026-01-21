#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 in_position;

void main()
{
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 1.0f);
    //gl_Position = vec4(in_position.xy, 0.5, 1.0);

    // gl_Position = vec4(
    //        (gl_VertexIndex == 0) ? -1.0 : 3.0,
    //        (gl_VertexIndex == 2) ? -1.0 : 3.0,
    //        0.5,
    //        1.0
    //    );
}
