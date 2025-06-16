#version 450

layout(location = 0) in vec4 in_pos; // position + size (w = scale)
layout(location = 1) in vec4 in_tint; // tint color
layout(location = 2) in vec4 in_colour; // actual light color (could be used in fragment shader)

layout(location = 0) out vec2 frag_offset;
layout(location = 1) out vec4 frag_color;

struct _point_light {
    vec4 pos;
    vec4 tint;
    vec4 colour;
};

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    vec4 ambient_light;
    _point_light lights[16]; // this is still needed for fragment lighting, if used
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
    frag_color = in_tint;

    vec3 right = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
    vec3 up = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);

    vec3 world_pos =
        in_pos.xyz
            + frag_offset.x * in_pos.w * 0.5 * right
            + frag_offset.y * in_pos.w * 0.5 * up;

    gl_Position = ubo.proj * ubo.view * vec4(world_pos, 1.0);
}
