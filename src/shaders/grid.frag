#version 450

layout(location = 0) in vec3 near_point;
layout(location = 1) in vec3 far_point;

layout(location = 0) out vec4 frag_color;

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    mat4 inv_proj;
    mat4 inv_view;
    vec4 ambient;
} ubo;

vec4 grid(vec3 pos, float scale) {
    vec2 coord = pos.xz * scale;
    vec2 d = fwidth(coord);
    vec2 g = abs(fract(coord - 0.5) - 0.5) / d;
    float line = min(g.x, g.y);
    float alpha = 1.0 - min(line, 1.0);

    vec3 color = vec3(0.35);
    return vec4(color, alpha);
}

float compute_depth(vec3 pos) {
    vec4 clip = ubo.proj * ubo.view * vec4(pos, 1.0);
    return clip.z / clip.w;
}

void main() {
    const float CELL_SCALE = 2.0;
    const float LOD_BASE = 2.0;
    const float LOD_OFFSET = -0.7;
    const float MIN_CAM_DIST = 10.0;
    const vec3 GRID_COLOR = vec3(0.35);
    const float ALPHA_CUTOFF = 0.01;
    const float FADE_START = 1000.0;
    const float FADE_END = 1200.0;

    float t = -near_point.y / (far_point.y - near_point.y);
    if (t < 0.0) discard;
    vec3 pos = near_point + t * (far_point - near_point);
    vec3 cam = ubo.inv_view[3].xyz;

    float cam_dist = max(abs(cam.y), MIN_CAM_DIST);

    float lod = log(cam_dist) / log(LOD_BASE) + LOD_OFFSET;
    float cell = pow(LOD_BASE, floor(lod)) * CELL_SCALE;
    float cell2 = cell * LOD_BASE;

    vec4 ga = grid(pos, 1.0 / cell);
    vec4 gb = grid(pos, 1.0 / cell2);

    float blend = smoothstep(0.0, 1.0, fract(lod));
    float alpha = mix(ga.a, gb.a, blend);

    float frag_dist = length((pos - cam).xz);
    alpha *= 1.0 - smoothstep(FADE_START, FADE_END, frag_dist);

    if (alpha < ALPHA_CUTOFF) discard;

    gl_FragDepth = compute_depth(pos);
    frag_color = vec4(GRID_COLOR, alpha);
}
