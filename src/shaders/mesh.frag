#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 frag_pos;
layout(location = 1) in vec2 frag_uv;
layout(location = 2) in vec3 frag_norm;
layout(location = 3) in flat uvec4 frag_data;

layout(location = 0) out vec4 out_color;

struct _billboard {
    vec4 pos_w;
    vec4 size_rotation;
    uvec4 type_data;
    uvec4 flags;
};

struct _solar_object {
    vec3 position;
    vec3 velocity;
    vec3 acceleration;

    float mass;
    float radius;

    uint colour_id;
    uint billboard_index;
    uint type;
};

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    vec4 ambient;
} ubo;

layout(std430, binding = 1) readonly buffer _sbo_billboards {
    uint count;
    _billboard objects[];
} sbo_billboards;

layout(std430, binding = 2) readonly buffer _sbo_solar_objects {
    uint count;
    _solar_object objects[];
} sbo_solar;

//layout(binding = 3) uniform sampler2D tex_samplers[];

vec3 unpack_color(uint packed_color) {
    return vec3(
        float((packed_color >> 16) & 0xFF) / 255.0,
        float((packed_color >> 8) & 0xFF) / 255.0,
        float(packed_color & 0xFF) / 255.0
    );
}

void main() {
    vec3 diffuse_light = ubo.ambient.xyz * ubo.ambient.w;
    vec3 surface_normal = normalize(frag_norm);

    for (int i = 0; i < sbo.billboard_count; i++) {
        bool is_light = (sbo.billboards[i].flags.x & 1u) != 0u;
        if (!is_light) continue;

        bool is_hud = (sbo.billboards[i].flags.z & 1u) != 0u;
        if (is_hud) continue;

        vec3 light_pos = sbo.billboards[i].pos_w.xyz;
        vec3 light_direction = light_pos - frag_pos;

        float distance_sq = dot(light_direction, light_direction);
        float attenuation = 1.0 / distance_sq;

        light_direction = normalize(light_direction);

        float cos_ang_incidence = max(dot(surface_normal, light_direction), 0.0);

        vec3 light_color = unpack_color(sbo.billboards[i].type_data.x);
        float light_intensity = sbo.billboards[i].pos_w.w;

        vec3 intensity = light_color * light_intensity * attenuation;
        diffuse_light += intensity * cos_ang_incidence;
    }

    vec3 surface_color = unpack_color(frag_data[0]);
    vec3 diffuse_colour = diffuse_light * surface_color;

    out_color = vec4(diffuse_colour, 1.0);
}
