#version 450

layout(location = 0) in vec3 in_pos;

layout(location = 0) out float frag_intensity;
layout(location = 1) out vec3 frag_world_pos;

layout(set = 0, binding = 0) uniform _ubo {
    mat4 proj;
    mat4 view;
    mat4 inv_proj;
    mat4 inv_view;
    vec4 ambient;
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
    uint _pad5;
};

layout(std430, set = 0, binding = 2) readonly buffer _sbo_solar_objects {
    uint solar_object_count;
    uint _pad[3];
    _solar_object solar_objects[];
} sbo_solar_objects;

const float GRAVITY_SCALE = 25.0;
const float SOFTENING = 0.01;
const float MAX_DEPRESSION = 60.0;

float compute_displacement(vec2 xz) {
    float disp = 0.0;
    uint count = sbo_solar_objects.solar_object_count;
    for (uint i = 0u; i < count; ++i) {
        _solar_object obj = sbo_solar_objects.solar_objects[i];
        float r = length(xz - obj.position.xz);
        disp -= GRAVITY_SCALE * obj.mass / (r + SOFTENING);
    }
    return max(disp, -MAX_DEPRESSION);
}

void main() {
    vec2 xz = in_pos.xz;
    float disp = compute_displacement(xz);
    vec3 world = vec3(xz.x, disp, xz.y);

    frag_intensity = clamp(-disp / 10.0, 0.0, 1.0);
    frag_world_pos = world;
    gl_Position = ubo.proj * ubo.view * vec4(world, 1.0);
}
