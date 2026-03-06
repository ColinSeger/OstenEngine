#version 450
#define MAX_LIGHTS 8

layout(set = 2, binding = 0) uniform sampler2D textures[20];

layout(set = 2, binding = 1) uniform sampler2D shadow_maps[MAX_LIGHTS];

layout(set = 2, binding = 2) uniform LightParams {
    vec3 light_dirs[MAX_LIGHTS];
} lights;

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 frag_tex_cord;
layout(location = 2) in vec4 frag_pos_light_space;

layout(location = 0) out vec4 out_color;

layout( push_constant ) uniform constants{
	uint texture_index;
} push_constants;

const float AMBIENT = 0.08;
const float SPECULAR_STRENGTH = 0.5;
const float SHININESS = 32.0;

float compute_shadow_factor(vec4 light_space_pos, sampler2D shadow_map1)
{
    // Convert light space position to NDC
    vec3 light_space_ndc = light_space_pos.xyz / light_space_pos.w;

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
    vec3 total_lighting = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        float NdotL = max(dot(frag_normal, normalize(lights.light_dirs[i])), 0.0);
        float shadow = compute_shadow_factor(frag_pos_light_space, shadow_maps[i]);
        total_lighting += (AMBIENT + shadow) * NdotL * texture(textures[push_constants.texture_index], frag_tex_cord).rgb;
    }

    out_color = vec4(total_lighting, 1.0);
}
