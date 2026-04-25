#version 450

layout(location = 0) out vec3 frag_world_pos;

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

const uint GRID_DIM = 256u;
const float GRID_EXTENT = 400.0;
const float TARGET_Y = 5.0;
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
    const ivec2 quad_offsets[6] = ivec2[](
            ivec2(0, 0), ivec2(1, 0), ivec2(1, 1),
            ivec2(0, 0), ivec2(1, 1), ivec2(0, 1)
        );

    uint vid = uint(gl_VertexIndex);
    uint quad_id = vid / 6u;
    uint local = vid % 6u;
    uint qx = quad_id % GRID_DIM;
    uint qz = quad_id / GRID_DIM;
    ivec2 o = quad_offsets[local];

    float cell = GRID_EXTENT / float(GRID_DIM);

    // snap grid origin to camera in cell-sized steps so the mesh follows the
    // viewer while keeping world-space alignment for grid lines and warping
    vec3 cam = ubo.inv_view[3].xyz;
    vec2 origin = floor(cam.xz / cell) * cell;

    vec2 local_xz = (vec2(float(qx + uint(o.x)), float(qz + uint(o.y)))
            - vec2(float(GRID_DIM) * 0.5)) * cell;
    vec2 xz = origin + local_xz;

    float y = TARGET_Y + compute_displacement(xz);
    vec3 world = vec3(xz.x, y, xz.y);

    frag_world_pos = world;
    gl_Position = ubo.proj * ubo.view * vec4(world, 1.0);
}
