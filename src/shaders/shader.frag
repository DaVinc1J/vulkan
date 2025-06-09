#version 450

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_normal;

layout(location = 0) out vec4 out_colour;

void main() {
    out_colour = texture(tex_sampler, frag_tex_coord);
}
