#version 450
layout(set = 2, binding = 0) uniform sampler2D textures;

layout(set = 2, binding = 1) uniform sampler2D shadow_map;

layout(set = 2, binding = 2) uniform LightParams {
    vec3 light_dir;
} light;

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 frag_tex_cord;
layout(location = 2) in vec4 frag_pos_light_space;

layout(location = 0) out vec4 out_color;

const float AMBIENT = 0.08;

float compute_shadow_factor(vec4 light_space_pos, sampler2D shadow_map1)
{
    // Convert light space position to NDC
    vec3 light_space_ndc = frag_pos_light_space.xyz / frag_pos_light_space.w;

    if (light_space_ndc.x > 1.0 ||
            light_space_ndc.y > 1.0 ||
            light_space_ndc.z > 1.0)
        return 0.0;

    // Translate from NDC to shadow map space (Vulkan's Z is already in [0..1])
    vec2 shadow_map_coord = light_space_ndc.xy * 0.5 + 0.5;

    // Check if the sample is in the light or in the shadow
    if (light_space_ndc.z > texture(shadow_map1, shadow_map_coord.xy).x)
        return 0.0; // In the shadow

    // In the light
    return 1.0;
}

void main()
{
    vec3 albedo = texture(textures, frag_tex_cord).rgb;
    vec3 normal = normalize(frag_normal); //Do I even need to do this?

    float dont_know_name = max(dot(normal, light.light_dir), 0.0);

    float shadow = compute_shadow_factor(frag_pos_light_space, shadow_map);

    vec3 lighting = AMBIENT + ((albedo * dont_know_name) * shadow);

    out_color = vec4(lighting, 1);
}
