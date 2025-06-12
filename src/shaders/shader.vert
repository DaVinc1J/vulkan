#version 450

layout(binding = 0) uniform _ubo {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_tex_coord;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in int in_tex_index;

layout(location = 0) out vec2 frag_tex_coord;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out flat int frag_tex_index;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 1.0);
    frag_tex_coord = in_tex_coord;
    frag_normal = mat3(ubo.model) * in_normal;
    frag_tex_index = in_tex_index;
}
