#version 450

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_data;
layout(location = 2) in vec4 in_flags;

layout(location = 0) out vec2 frag_offset;
layout(location = 1) out vec4 frag_color;

struct _billboard {
    vec4 pos;
    vec4 data;
    vec4 flags;
};

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    vec4 ambient_light;
    _billboard lights[16];
    int light_count;
} ubo;

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
    frag_color = in_data;

    vec3 right = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
    vec3 up = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);

    vec3 world_pos =
        in_pos.xyz
            + frag_offset.x * in_pos.w * 0.5 * right
            + frag_offset.y * in_pos.w * 0.5 * up;

    gl_Position = ubo.proj * ubo.view * vec4(world_pos, 1.0);
}
