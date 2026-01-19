#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_cord;

layout (location = 0) out vec2 out_uv;

void main()
{
//	out_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	//gl_Position = vec4(out_uv * 2.0f - 1.0f, 0.0f, 1.0f);
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 1.0f);
}
