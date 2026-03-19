#version 450
#define MAX_LIGHTS 8

layout(set = 2, binding = 0) uniform sampler2D textures[20];

layout(set = 2, binding = 1) uniform sampler2D shadow_maps[MAX_LIGHTS];

layout(set = 2, binding = 2) uniform LightParams {
    vec4 light_positions[MAX_LIGHTS];
} lights;

layout(location = 0) in vec3 frag_position;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec2 frag_tex_cord;
layout(location = 3) in vec4 frag_pos_light_space[MAX_LIGHTS];

layout(location = 0) out vec4 out_color;

layout( push_constant ) uniform constants{
	uint texture_index;
	float ambient;
	float specular;
	float shininess;
	vec3 camera_position;
	float light_amount;
} push_constants;

const float AMBIENT = 0.05;
const float SPECULAR_STRENGTH = 0.5;
const float SHININESS = 64.0;

float compute_shadow_factor(vec4 light_space_pos, sampler2D shadow_map1){
    vec3 projected_cords = light_space_pos.xyz / light_space_pos.w;

	if(projected_cords.z > 1.0 || projected_cords.z  < 0.0)
		return 0.0;

    vec2 shadow_map_coord = projected_cords.xy * 0.5 + 0.5;

	if(shadow_map_coord.x < 0.0 || shadow_map_coord.x > 1.0 ||
	   shadow_map_coord.y < 0.0 || shadow_map_coord.y > 1.0)
		return 0.0;

    float closest = texture(shadow_map1, shadow_map_coord.xy).x;

    float current = projected_cords.z;

	//float bias = 0.005;
	//float shadow = current - bias > closest ? 1.0 : 0.0;

    float shadow = current > closest  ? 0.0 : 1.0;

    return shadow;
}

void main(){
    vec3 albedo = texture(textures[push_constants.texture_index], frag_tex_cord).rgb;
    vec3 normal = normalize(frag_normal);

	vec3 lighting = albedo * push_constants.ambient;

    for(int i = 0; i < int(push_constants.light_amount); i++)
    {
        vec3 light_dir = normalize(lights.light_positions[i].xyz - frag_position);

        float diff = max(dot(normal, light_dir), 0.0);
        vec3 diffuse = diff * albedo;

        vec3 view_dir = normalize(push_constants.camera_position - frag_position);
        vec3 halfway_dir = normalize(light_dir + view_dir);

        float specular = push_constants.specular * pow(max(dot(normal, halfway_dir), 0.0), push_constants.shininess);

        float shadow = compute_shadow_factor(frag_pos_light_space[i], shadow_maps[i]);

        lighting += (shadow) * (diffuse + specular);
    }

    out_color = vec4(lighting, 1);
	//out_color = vec4(vec3(diff), 1.0);
	//out_color = vec4(vec3(shadow), 1.0);
	//out_color = texture(shadow_maps[0], frag_tex_cord);
}
