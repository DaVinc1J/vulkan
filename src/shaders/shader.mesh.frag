#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 frag_pos;
layout(location = 1) in vec3 frag_norm;
layout(location = 2) in vec2 frag_uv;
layout(location = 3) flat in int frag_tex_index;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec4 light_position;
    vec4 light_colour;
    vec4 ambient_light;
} ubo;

layout(binding = 1) uniform sampler2D tex_samplers[];

void main() {
    vec3 light_direction = ubo.light_position.xyz - frag_pos;
    float attenuation = 1.0 / dot(light_direction, light_direction);

    vec3 light_colour = ubo.light_colour.xyz * ubo.light_colour.w * attenuation;
    vec3 ambient_light = ubo.ambient_light.xyz * ubo.ambient_light.w;
    vec3 diffuse_light = light_colour * max(dot(normalize(frag_norm), normalize(light_direction)), 0.0);

    vec4 tex_colour = texture(tex_samplers[nonuniformEXT(frag_tex_index)], frag_uv);
    vec3 colour = (diffuse_light + ambient_light) * tex_colour.rgb;

    out_color = vec4(colour, tex_colour.a);
}
