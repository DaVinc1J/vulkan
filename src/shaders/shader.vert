#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex;
layout(location = 2) in vec3 in_norm;
layout(location = 3) in int tex_index;

layout(location = 0) out vec3 frag_pos;
layout(location = 1) out vec3 frag_norm;
layout(location = 2) out vec2 frag_uv;
layout(location = 3) flat out int frag_tex_index;

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 light_pos;
    float _padding;
} ubo;

void main() {
    vec4 world_pos = ubo.model * vec4(in_pos, 1.0);
    gl_Position = ubo.proj * ubo.view * world_pos;

    frag_pos = world_pos.xyz;
    frag_norm = mat3(transpose(inverse(ubo.model))) * in_norm;
    frag_uv = in_tex;
    frag_tex_index = tex_index;
}
