#version 450

layout(binding = 1) uniform sampler2D samplerColor;

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 out_frag_color;

// layout(binding = 0) uniform UBO
// {
//     mat4 projection;
//     mat4 view;
//     mat4 model;
//     mat4 lightSpace;
//     vec4 lightPos;
//     float zNear;
//     float zFar;
// } ubo;

float LinearizeDepth(float depth)
{
    float n = 0.1f;
    float f = 2000.f;
    float z = depth;
    return (2.0 * n) / (f + n - z * (f - n));
}

void main()
{
    float depth = texture(samplerColor, inUV).r;
    out_frag_color = vec4(vec3(1.0 - LinearizeDepth(depth)), 1.0);
}
