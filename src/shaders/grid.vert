#version 450

layout(location = 0) out vec3 near_point;
layout(location = 1) out vec3 far_point;

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    mat4 inv_proj;
    mat4 inv_view;
    vec4 ambient;
} ubo;

vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2(3.0, -1.0),
        vec2(-1.0, 3.0)
    );

vec3 unproject(vec3 ndc) {
    vec4 w = ubo.inv_view * ubo.inv_proj * vec4(ndc, 1.0);
    return w.xyz / w.w;
}

void main() {
    vec2 p = positions[gl_VertexIndex];
    near_point = unproject(vec3(p, 0.0));
    far_point = unproject(vec3(p, 1.0));
    gl_Position = vec4(p, 0.0, 1.0);
}
