#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in float in_size;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec2 fragOffset;
layout(location = 1) out vec4 frag_color;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 proj;
    mat4 view;
    vec4 ambientLightColor;
    vec4 lightPosition;
    vec4 lightColor;
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
    fragOffset = OFFSETS[gl_VertexIndex];
    frag_color = in_color;

    vec3 right = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
    vec3 up = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);

    vec3 world_pos = in_position
            + fragOffset.x * in_size * 0.5 * right
            + fragOffset.y * in_size * 0.5 * up;

    gl_Position = ubo.proj * ubo.view * vec4(world_pos, 1.0);
}
