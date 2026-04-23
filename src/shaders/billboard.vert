#version 450

layout(location = 0) in vec4 in_pos_w;
layout(location = 1) in vec4 in_size_rotation;
layout(location = 2) in uvec4 in_type_data;
layout(location = 3) in uvec4 in_flags;

layout(location = 0) out vec2 frag_offset;
layout(location = 1) out vec4 frag_size_rotation;
layout(location = 2) out flat uvec4 frag_type_data;
layout(location = 3) out flat uvec4 frag_flags;
layout(location = 4) out float frag_alpha;

struct _billboard {
    vec4 pos_w;
    vec4 size_rotation;
    uvec4 type_data;
    uvec4 flags;
};

struct _solar_object {
    vec3 position;
    float _pad0;
    vec3 velocity;
    float _pad1;
    vec3 acceleration;
    float _pad2;

    float mass;
    float radius;

    uint colour_id;
    uint billboard_index;
    uint type;
    uint _pad3;
};

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    vec4 ambient;
} ubo;

layout(std430, binding = 1) readonly buffer _sbo_billboards {
    uint billboard_count;
    uint _pad[3];
    _billboard billboards[];
} sbo_billboards;

layout(std430, binding = 2) readonly buffer _sbo_solar_objects {
    uint solar_object_count;
    uint _pad[3];
    _solar_object solar_objects[];
} sbo_solar_objects;

const vec2 OFFSETS[6] = vec2[](
        vec2(-1.0, -1.0),
        vec2(1.0, -1.0),
        vec2(1.0, 1.0),
        vec2(-1.0, -1.0),
        vec2(1.0, 1.0),
        vec2(-1.0, 1.0)
    );

void main() {
    frag_offset = OFFSETS[gl_VertexIndex];
    frag_size_rotation = in_size_rotation;
    frag_type_data = in_type_data;
    frag_flags = in_flags;
    frag_alpha = in_pos_w.w;

    bool is_hud = (in_flags[2] & 1u << 0) == 1u;

    if (is_hud) {
        vec2 screen_pos = in_pos_w.xy + (frag_offset * 0.5 + 0.5) * in_size_rotation.xy;

        screen_pos = screen_pos * 2.0 - 1.0;

        screen_pos.y = -screen_pos.y;

        gl_Position = vec4(screen_pos, 0.0, 1.0);
    } else {
        vec3 right = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
        vec3 up = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);

        vec3 world_pos =
            in_pos_w.xyz
                + frag_offset.x * in_size_rotation.x * 0.5 * right
                + frag_offset.y * in_size_rotation.y * 0.5 * up;

        gl_Position = ubo.proj * ubo.view * vec4(world_pos, 1.0);
    }
}
