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

float compute_shadow_factor(vec4 light_space_pos, sampler2D shadow_map1){
    vec3 projected_cords = light_space_pos.xyz /= light_space_pos.w;

    vec2 shadow_map_coord = projected_cords.xy * 0.5 + 0.5;

    float closest = texture(shadow_map1, shadow_map_coord.xy).x;

    float current = projected_cords.z;

    float shadow = current > closest  ? 0 : 1;

    return shadow;
}

void main()
{
    vec3 albedo = texture(textures[push_constants.texture_index], frag_tex_cord).rgb;
    vec3 normal = normalize(frag_normal);

    float dont_know_name = max(dot(normal, lights.light_dirs[0]), 0.0);

    float shadow = compute_shadow_factor(frag_pos_light_space, shadow_maps[0]);

    vec3 lighting = (AMBIENT + (shadow)) * dont_know_name * albedo;

    out_color = vec4(lighting, 1);
}
