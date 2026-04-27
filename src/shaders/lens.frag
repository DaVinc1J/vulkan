#version 450

layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 out_colour;

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    mat4 inv_proj;
    mat4 inv_view;
    vec4 ambient;
    vec4 grid_params;
} ubo;

struct _solar_object {
    vec3 position;
    float _pad0;
    vec3 velocity;
    float _pad1;
    vec3 acceleration;
    float _pad2;
    float mass;
    float radius;
    uint colour_id;
    uint billboard_index;
    uint type;
    uint planet_type;
    float intensity;
    float schwarzschild_radius;
};

layout(std430, binding = 1) readonly buffer _sbo_solar_objects {
    uint solar_object_count;
    uint _pad[3];
    _solar_object objects[];
} sbo_solar_objects;

layout(set = 0, binding = 2) uniform sampler2D scene_tex;

const uint SOLAR_OBJECT_TYPE_BLACKHOLE = 3u;
const float INFLUENCE_FACTOR = 50.0;
const int MAX_RK4_STEPS = 256;
const float ESCAPE_U_FACTOR = 0.005;
const float DPHI_BASE = 0.025;

// Schwarzschild null geodesic in (u = 1/r, phi):
//   d^2 u / dphi^2 + u = 1.5 * rs * u^2
vec2 geodesic_rhs(vec2 s, float rs) {
    return vec2(s.y, 1.5 * rs * s.x * s.x - s.x);
}
vec2 rk4_step(vec2 s, float h, float rs) {
    vec2 k1 = geodesic_rhs(s, rs);
    vec2 k2 = geodesic_rhs(s + 0.5 * h * k1, rs);
    vec2 k3 = geodesic_rhs(s + 0.5 * h * k2, rs);
    vec2 k4 = geodesic_rhs(s + h * k3, rs);
    return s + (h / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

vec3 starfield(vec3 d) {
    float phi = atan(d.z, d.x);
    float theta = acos(clamp(d.y, -1.0, 1.0));
    vec2 sky = vec2(phi * 0.15915494, theta * 0.31830989);
    vec2 cell = floor(sky * 512.0);
    float h = fract(sin(dot(cell, vec2(127.1, 311.7))) * 43758.5453);
    float star = pow(h, 80.0) * 6.0;
    return vec3(star) + vec3(0.01, 0.012, 0.025);
}

void main() {
    vec2 ndc = v_uv * 2.0 - 1.0;
    vec4 view_h = ubo.inv_proj * vec4(ndc, 1.0, 1.0);
    vec3 view_dir = view_h.xyz / view_h.w;
    vec3 world_dir = normalize((ubo.inv_view * vec4(view_dir, 0.0)).xyz);
    vec3 cam_pos = (ubo.inv_view * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    int best_bh = -1;
    float best_score = 0.0;
    for (uint i = 0u; i < sbo_solar_objects.solar_object_count; ++i) {
        _solar_object obj = sbo_solar_objects.objects[i];
        if (obj.type != SOLAR_OBJECT_TYPE_BLACKHOLE) continue;

        vec3 to_bh = obj.position - cam_pos;
        float r0 = length(to_bh);
        if (r0 < obj.schwarzschild_radius) {
            out_colour = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
        float fwd = dot(to_bh, world_dir);
        if (fwd <= 0.0) continue;
        vec3 perp = to_bh - fwd * world_dir;
        float b = length(perp);
        if (b > obj.schwarzschild_radius * INFLUENCE_FACTOR) continue;
        float score = obj.schwarzschild_radius / max(b * b, 1e-6);
        if (score > best_score) {
            best_score = score;
            best_bh = int(i);
        }
    }

    if (best_bh < 0) {
        out_colour = texture(scene_tex, v_uv);
        return;
    }

    _solar_object bh = sbo_solar_objects.objects[best_bh];
    float rs = bh.schwarzschild_radius;

    vec3 to_bh = bh.position - cam_pos;
    float r0 = length(to_bh);
    vec3 e_r = -to_bh / r0;
    vec3 plane_n = cross(e_r, world_dir);
    float n_len = length(plane_n);
    if (n_len < 1e-6) {
        if (dot(to_bh, world_dir) > 0.0) {
            out_colour = vec4(0.0, 0.0, 0.0, 1.0);
        } else {
            out_colour = texture(scene_tex, v_uv);
        }
        return;
    }
    plane_n /= n_len;
    vec3 e_phi = cross(plane_n, e_r);

    float v_r = dot(world_dir, e_r);
    float v_phi = dot(world_dir, e_phi);
    float u0 = 1.0 / r0;
    float du0 = -v_r / (r0 * v_phi);
    vec2 state = vec2(u0, du0);

    float phi = 0.0;
    bool absorbed = false;
    for (int step = 0; step < MAX_RK4_STEPS; ++step) {
        if (state.x * rs >= 1.0) {
            absorbed = true;
            break;
        }
        if (state.x * rs < ESCAPE_U_FACTOR && state.y < 0.0) break;

        float local_dphi = DPHI_BASE / max(state.x * rs * 5.0, 1.0);
        state = rk4_step(state, local_dphi, rs);
        phi += local_dphi;
    }

    if (absorbed) {
        out_colour = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float r_end = 1.0 / max(state.x, 1e-6);
    float dr_dphi = -state.y / max(state.x * state.x, 1e-6);
    float cphi = cos(phi);
    float sphi = sin(phi);
    vec3 e_rad = cphi * e_r + sphi * e_phi;
    vec3 e_tan = -sphi * e_r + cphi * e_phi;
    vec3 bent_dir = normalize(dr_dphi * e_rad + r_end * e_tan);

    vec3 far_pt = cam_pos + bent_dir * 1000.0;
    vec4 clip = ubo.proj * ubo.view * vec4(far_pt, 1.0);
    if (clip.w <= 0.0) {
        out_colour = vec4(starfield(bent_dir), 1.0);
        return;
    }
    vec2 ndc_out = clip.xy / clip.w;
    vec2 uv_out = ndc_out * 0.5 + 0.5;

    if (any(lessThan(uv_out, vec2(0.0))) || any(greaterThan(uv_out, vec2(1.0)))) {
        out_colour = vec4(starfield(bent_dir), 1.0);
        return;
    }

    out_colour = texture(scene_tex, uv_out);
}
