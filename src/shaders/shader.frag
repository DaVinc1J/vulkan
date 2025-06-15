#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 frag_pos;
layout(location = 1) in vec3 frag_norm;
layout(location = 2) in vec2 frag_uv;
layout(location = 3) flat in int frag_tex_index;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 light_pos;
    float _padding;
} ubo;

layout(binding = 1) uniform sampler2D tex_samplers[];

void main() {
    vec3 normal = normalize(frag_norm);
    vec3 light_dir = normalize(ubo.light_pos - frag_pos);
    float diff = max(dot(normal, light_dir), 0.0);

    vec4 tex_color = texture(tex_samplers[nonuniformEXT(frag_tex_index)], frag_uv);
    vec3 lit_color = tex_color.rgb * diff;

    out_color = vec4(lit_color, tex_color.a);
}
