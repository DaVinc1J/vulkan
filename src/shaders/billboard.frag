#version 450

layout(location = 0) in vec2 frag_offset;
layout(location = 1) in vec4 frag_colour;
layout(location = 0) out vec4 out_colour;

void main() {
    float dist = sqrt(dot(frag_offset, frag_offset));

    float inner = 0.7;
    float outer = 1.0;

    if (dist > outer) discard;

    float alpha = 1.0;

    if (dist > inner) {
        float t = (dist - inner) / (outer - inner);
        alpha = 1.0 - t * t;
    }

    out_colour = vec4(frag_colour.rgb, frag_colour.a * alpha);
}
