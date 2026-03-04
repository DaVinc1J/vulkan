#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_norm;
layout(location = 3) in uvec4 in_data;

layout(location = 0) out vec3 frag_pos;
layout(location = 1) out vec2 frag_uv;
layout(location = 2) out vec3 frag_norm;
layout(location = 3) out uvec4 frag_data;

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
    uint billboard_count;
    _billboard billboards[];
} sbo_billboards;

layout(std430, binding = 2) readonly buffer _sbo_solar_objects {
    uint solar_object_count;
    _solar_object solar_objects[];
} sbo_solar_objects;

void main() {
    gl_Position = ubo.proj * ubo.view * vec4(in_pos, 1.0);

    frag_pos = in_pos;
    frag_norm = normalize(in_norm);

    frag_uv = in_uv;
    frag_data = in_data;
}
