#include "headers/app.h"

void app_init(_app *p_app) {
	p_app->config.win_title = "davincij";
	p_app->config.win_width = 800;
	p_app->config.win_height = 800;

	p_app->sync.frame_index = 0;
	p_app->atlas.scale = 1024;
	p_app->atlas.max_scale = 16384;
	p_app->atlas.flags = PACKER_FLAG_ALWAYS_REGENERATE;

	p_app->shader.mesh_vert = "src/shaders/mesh.vert.spv";
	p_app->shader.mesh_frag = "src/shaders/mesh.frag.spv";
	p_app->shader.billboard_vert = "src/shaders/billboard.vert.spv";
	p_app->shader.billboard_frag = "src/shaders/billboard.frag.spv";

	char *gltf_files[] = {
		"doom_E1M1.gltf"
	};

	p_app->tex.atlas.count = 0;
	p_app->tex.file.count = sizeof(gltf_files) / sizeof(gltf_files[0]);
	p_app->tex.file.names = malloc(sizeof(char *) * p_app->tex.file.count);
	for (u32 i = 0; i < p_app->tex.file.count; i++) {
		p_app->tex.file.names[i] = strdup(gltf_files[i]);
	}

	p_app->config.dir.objects = "src/objects/";
	p_app->config.dir.atlases = "src/atlases/";
	p_app->config.dir.textures = "src/textures/";

	glm_vec3_copy((vec3){0.0f, 2.0f, 0.0f}, p_app->view.camera_pos);
	glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, p_app->view.target);
	glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, p_app->view.world_up);
	p_app->view.fov_y = 80.0f;
	p_app->view.near_plane = 0.01f;
	p_app->view.far_plane = 1000.0f;
	p_app->view.rotation_speed = 0.0f;
	p_app->view.speed = 0.1f;
	p_app->view.lerp_speed = 1.0f;
	p_app->view.sensitivity = 0.2f;
	p_app->view.yaw = -90.0f;
	p_app->view.pitch = 0.0f;
	p_app->view.first_mouse = true;
	p_app->view.mouse_locked = true;

	static const _billboard lights[] = {
		{ .pos = {0.0f, 1.0f, 0.0f, 0.25f}, .data = {1.0f, 1.0f, 0.0f, 0.5f}, .flags = {0, 0, 0, 0}},
		{ .pos = {0.0f, 1.0f, 0.0f, 0.25f}, .data = {1.0f, 0.0f, 1.0f, 0.5f}, .flags = {0, 0, 0, 0}},
		{ .pos = {0.0f, 1.0f, 0.0f, 0.25f}, .data = {0.0f, 1.0f, 1.0f, 0.5f}, .flags = {0, 0, 0, 0}},
	};

	p_app->obj.light_count = sizeof(lights) / sizeof(lights[0]);
	p_app->obj.lights = malloc(sizeof(_billboard) * p_app->obj.light_count);
	memcpy(p_app->obj.lights, lights, sizeof(lights));

	glm_vec4_copy((vec4){1.0f, 1.0f, 1.0f, 0.0f}, p_app->lighting.ambient);
}
