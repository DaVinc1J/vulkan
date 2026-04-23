#include "headers/app.h"

void app_init(_app *p_app) {

	p_app->config.win_title = "davincij";
	p_app->config.win_width = 800;
	p_app->config.win_height = 800;

	p_app->sync.frame_index = 0;

	p_app->config.flags = CONFIG_FLAG_NONE;

	p_app->shader.mesh_vert = "src/shaders/mesh.vert.spv";
	p_app->shader.mesh_frag = "src/shaders/mesh.frag.spv";
	p_app->shader.billboard_vert = "src/shaders/billboard.vert.spv";
	p_app->shader.billboard_frag = "src/shaders/billboard.frag.spv";

	glm_vec3_copy((vec3){0.0f, 0.0f, 1.0f}, p_app->view.camera_pos);
	glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, p_app->view.target);
	glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, p_app->view.world_up);
	p_app->view.fov_y = 80.0f;
	p_app->view.near_plane = 0.1f;
	p_app->view.far_plane = 100.0f;
	p_app->view.rotation_speed = 0.0f;
	p_app->view.speed = 0.5f;
	p_app->view.lerp_speed = 0.1f;
	p_app->view.sensitivity = 0.5f;
	p_app->view.yaw = -90.0f;
	p_app->view.pitch = 0.0f;
	p_app->view.first_mouse = true;
	p_app->view.mouse_locked = true;

	static const _solar_object solar_objects[] = {
		{
			.position = {0.0f, 5.0f, 0.0f},
			.velocity = {0.0f, 0.0f, 0.0f},

			.mass = 4.0e9f,
			.radius = 1.0f,
			.colour_id = COLOUR_WHITE,
			.type = SOLAR_OBJECT_TYPE_PLAIN,
		},
		{
			.position = {-10.0f, 0.0f, -10.0f},
			.velocity = {-2.0f, 0.0f, 2.0f},

			.mass = 1.0e5f,
			.radius = 0.5f,
			.colour_id = COLOUR_PURPLE,
			.type = SOLAR_OBJECT_TYPE_LIGHT_EMIT,
		},
		{
			.position = {10.0f, 0.0f, 10.0f},
			.velocity = {2.0f, 0.0f, -2.0f},

			.mass = 1.0e5f,
			.radius = 0.5f,
			.colour_id = COLOUR_BLUE,
			.type = SOLAR_OBJECT_TYPE_LIGHT_EMIT,
		},
		{
			.position = {-10.0f, 0.0f, 10.0f},
			.velocity = {2.0f, 0.0f, 2.0f},

			.mass = 1.0e5f,
			.radius = 0.5f,
			.colour_id = COLOUR_RED,
			.type = SOLAR_OBJECT_TYPE_LIGHT_EMIT,
		},
		{
			.position = {10.0f, 0.0f, -10.0f},
			.velocity = {-2.0f, 0.0f, -2.0f},

			.mass = 1.0e5f,
			.radius = 0.5f,
			.colour_id = COLOUR_YELLOW,
			.type = SOLAR_OBJECT_TYPE_LIGHT_EMIT,
		},
	};


	p_app->obj.primitive_count = 0;

	p_app->obj.billboards = NULL;
	p_app->obj.billboard_count = 0;
	p_app->obj.billboard_max = 0;

	p_app->obj.solar_object_count = sizeof(solar_objects) / sizeof(solar_objects[0]);
	p_app->obj.solar_objects = malloc(sizeof(_solar_object) * p_app->obj.solar_object_count);
	memcpy(p_app->obj.solar_objects, solar_objects, sizeof(solar_objects));

	glm_vec4_copy((vec4){1.0f, 1.0f, 1.0f, 0.2f}, p_app->lighting.ambient);
}
