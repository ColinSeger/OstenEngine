#version 450
layout(set = 1, binding = 0) uniform sampler2D textures[]; //Index 0 is shadowmap index 1 is texture

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec2 frag_tex_cord;
layout(location = 2) in vec4 frag_pos_light_space;

layout(location = 0) out vec4 out_color;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(10.0, 0.0, 2.0));
const float AMBIENT = 0.08;

float calculate_shadow(vec4 frag_pos_light_space, vec3 normal)
{
    // Perspective divide
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;

    // Transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;

    // Outside shadow map → no shadow
    if (proj_coords.z > 1.0)
        return 0.0;

    float current_depth = proj_coords.z;

    // Bias to prevent shadow acne
    float bias = max(0.005 * (1.0 - dot(normal, -DIRECTION_TO_LIGHT)), 0.0005);

    // Manual PCF (3x3)
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(textures[0], 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closest_depth = texture(
                    textures[0],
                    proj_coords.xy + vec2(x, y) * texel_size
                ).r;

            shadow += current_depth - bias > closest_depth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 albedo = texture(textures[1], frag_tex_cord).rgb;
    vec3 normal = normalize(frag_normal);

    // Lighting
    float diff = max(dot(normal, -DIRECTION_TO_LIGHT), 0.0);
    vec3 diffuse = diff * vec3(1);

    float shadow = calculate_shadow(frag_pos_light_space, frag_normal);

    vec3 lighting = (AMBIENT + (1.0 - shadow) * diffuse) * albedo;

    out_color = vec4(lighting, 1.0);
}
