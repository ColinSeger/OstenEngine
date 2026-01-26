#version 450
layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 frag_tex_cord;

layout(location = 0) out vec4 out_color;

float compute_shadow_factor(vec4 light_space_pos, sampler2D shadow_map)
{
    // Convert light space position to NDC
    vec3 light_space_ndc = light_space_pos.xyz /= light_space_pos.w;

    // If the fragment is outside the light's projection then it is outside
    // the light's influence, which means it is in the shadow (notice that
    // such sample would be outside the shadow map image)
    if (abs(light_space_ndc.x) > 1.0 ||
            abs(light_space_ndc.y) > 1.0 ||
            abs(light_space_ndc.z) > 1.0)
        return 0.0;

    // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
    vec2 shadow_map_coord = light_space_ndc.xy * 0.5 + 0.5;

    // Check if the sample is in the light or in the shadow
    if (light_space_ndc.z > texture(shadow_map, shadow_map_coord.xy).x)
        return 0.0; // In the shadow

    // In the light
    return 1.0;
}

void main() {
    out_color = (vec4(frag_normal, 1) * texture(textures[0], frag_tex_cord)) * compute_shadow_factor(vec4(frag_normal, 1), textures[0]);
}
