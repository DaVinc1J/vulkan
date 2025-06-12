#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 2) in flat int frag_tex_index;
layout(binding = 1) uniform sampler2D tex_samplers[];

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_normal;

layout(location = 0) out vec4 out_colour;

void main() {
    out_colour = texture(tex_samplers[nonuniformEXT(frag_tex_index)], frag_tex_coord);
}
