#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 frag_pos;
layout(location = 1) in vec3 frag_norm;
layout(location = 2) in vec2 frag_uv;
layout(location = 3) flat in int frag_tex_index;

layout(location = 0) out vec4 out_color;

struct _billboard {
    vec4 pos;
    vec4 data;
    vec4 flags;
};

layout(binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    vec4 ambient_light;
    _billboard lights[16];
    int light_count;
} ubo;

layout(binding = 1) uniform sampler2D tex_samplers[];

void main() {
    vec3 diffuse_light = ubo.ambient_light.xyz * ubo.ambient_light.w;
    vec3 surface_normal = normalize(frag_norm);

    for (int i = 0; i < ubo.light_count; i++) {
        vec3 light_direction = ubo.lights[i].pos.xyz - frag_pos;
        float attenuation = 1.0 / dot(light_direction, light_direction);
        float cos_ang_incidence = max(dot(surface_normal, normalize(light_direction)), 0.0);
        vec3 intensity = ubo.lights[i].data.xyz * ubo.lights[i].data.w * attenuation;

        diffuse_light += intensity * cos_ang_incidence;
    }

    vec4 tex_colour = texture(tex_samplers[nonuniformEXT(frag_tex_index)], frag_uv);
    vec3 colour = diffuse_light * tex_colour.rgb;

    out_color = vec4(colour, tex_colour.a);
}
