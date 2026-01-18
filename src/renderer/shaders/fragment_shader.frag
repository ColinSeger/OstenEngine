#version 450
layout(binding = 1) uniform sampler2D texture_sampler;

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 frag_tex_cord;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(frag_normal, 1) * texture(texture_sampler, frag_tex_cord);
}
