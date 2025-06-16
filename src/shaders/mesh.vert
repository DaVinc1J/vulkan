#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex;
layout(location = 2) in vec3 in_norm;
layout(location = 3) in int tex_index;

layout(location = 0) out vec3 frag_pos;
layout(location = 1) out vec3 frag_norm;
layout(location = 2) out vec2 frag_uv;
layout(location = 3) flat out int frag_tex_index;

layout(push_constant) uniform _push_constants {
    mat4 model;
    mat4 normal;
} push;

struct _point_light {
    vec4 pos;
    vec4 tint;
    vec4 colour;
};

layout(binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    vec4 ambient_light;
    _point_light lights[16];
    int light_count;
} ubo;

void main() {
    vec4 world_pos = push.model * vec4(in_pos, 1.0);
    gl_Position = ubo.proj * ubo.view * world_pos;

    frag_pos = world_pos.xyz;
    frag_norm = mat3(push.normal) * in_norm;
    frag_uv = in_tex;
    frag_tex_index = tex_index;
}
