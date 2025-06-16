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

layout(binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec4 light_position;
    vec4 light_colour;
    vec4 ambient_light;
} ubo;

void main() {
    vec4 world_pos = push.model * vec4(in_pos, 1.0);
    gl_Position = ubo.proj * ubo.view * world_pos;

    frag_pos = world_pos.xyz;
    frag_norm = mat3(push.normal) * in_norm;
    frag_uv = in_tex;
    frag_tex_index = tex_index;
}
