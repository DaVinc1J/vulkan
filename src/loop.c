#include "headers/loop.h"
#include "headers/validation.h"
#include "headers/swapchain.h"
#include "headers/buffer.h"

void log_performance(_app *p_app) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	if (p_app->perf.frame_count == 0) {
		p_app->perf.last_frame_time = now;
		p_app->perf.frame_count++;
		return;
	}

	double dt = (now.tv_sec - p_app->perf.last_frame_time.tv_sec) +
		(now.tv_nsec - p_app->perf.last_frame_time.tv_nsec) / 1e9;

	p_app->perf.last_frame_time = now;
	p_app->perf.delta_time = (float)dt;

	p_app->perf.frame_time_avg += (dt - p_app->perf.frame_time_avg) * 0.05;
	p_app->perf.fps_avg = 1.0f / p_app->perf.frame_time_avg;

	p_app->perf.frame_count++;
	if (p_app->perf.frame_count % 60 == 0) {
		printf("[perf] FPS: %.1f, Frame Time: %.2f ms\n", p_app->perf.fps_avg, p_app->perf.frame_time_avg * 1000.0f);
	}
}

float get_delta_time() {
	static struct timespec last_time = {0};
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	float delta = (now.tv_sec - last_time.tv_sec) + 
		(now.tv_nsec - last_time.tv_nsec) / 1e9f;

	last_time = now;
	return delta;
}

void draw_frame(_app *p_app) {
	vkWaitForFences(p_app->device.logical, 1, &p_app->sync.in_flight_fences[p_app->sync.frame_index], VK_TRUE, UINT64_MAX);

	u32 image_index;
	VkResult aquire_result = vkAcquireNextImageKHR(
		p_app->device.logical,
		p_app->swp.swapchain,
		UINT64_MAX,
		p_app->sync.image_available_semaphores[p_app->sync.frame_index],
		VK_NULL_HANDLE,
		&image_index
	);

	if (aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain(p_app);
		return;
	} else if (aquire_result != VK_SUCCESS && aquire_result != VK_SUBOPTIMAL_KHR) {

		printf("vkAcquireNextImageKHR returned: %d, image index: %u\n", aquire_result, image_index);

		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to acquire swap chain image"
		);
		exit(EXIT_FAILURE);
	}


	update_uniform_buffer(p_app, p_app->sync.frame_index);

	vkResetFences(p_app->device.logical, 1, &p_app->sync.in_flight_fences[p_app->sync.frame_index]);

	vkResetCommandBuffer(p_app->cmd.buffers[p_app->sync.frame_index], 0);
	record_command_buffer(p_app, p_app->cmd.buffers[p_app->sync.frame_index], image_index);

	VkSemaphore wait_semaphores[] = {p_app->sync.image_available_semaphores[p_app->sync.frame_index]};
	VkSemaphore signal_semaphores[] = {p_app->sync.render_finished_semaphores[image_index]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &p_app->cmd.buffers[p_app->sync.frame_index],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores,
	};

	if (vkQueueSubmit(p_app->device.graphics_queue, 1, &submit_info, p_app->sync.in_flight_fences[p_app->sync.frame_index]) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"graphics queue => failed to submit next queue"
		);
		exit(EXIT_FAILURE);
	}

	VkSwapchainKHR swapchains[] = {p_app->swp.swapchain};
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &image_index,
		.pResults = NULL,
	};

	VkResult present_result = vkQueuePresentKHR(p_app->device.present_queue, &present_info);

	if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR || p_app->swp.framebuffer_resized) {
		p_app->swp.framebuffer_resized = false;
		recreate_swapchain(p_app);
		return;
	} else if (present_result != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to present swap chain image"
		);
		exit(EXIT_FAILURE);
	}

	p_app->sync.frame_index = (p_app->sync.frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

void update_uniform_buffer(_app *p_app, u32 current_image) {
	static struct timespec start_time;
	static bool initialised = false;
	if (!initialised) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		initialised = true;
	}

	float time = (p_app->perf.last_frame_time.tv_sec - start_time.tv_sec) +
		(p_app->perf.last_frame_time.tv_nsec - start_time.tv_nsec) / 1e9f;

	_ubo ubo;

	glm_lookat(p_app->view.camera_pos, p_app->view.target, p_app->view.world_up, ubo.view);

	float aspect = (float)p_app->swp.extent.width / (float)p_app->swp.extent.height;
	glm_perspective(glm_rad(p_app->view.fov_y), aspect,
								 p_app->view.near_plane, p_app->view.far_plane, ubo.proj);
	ubo.proj[1][1] *= -1;

	float radius = 1.0f;
	float speed = 3.0f;

	int light_count = p_app->obj.light_count;
	for (int i = 0; i < light_count; ++i) {
		float angle = speed * time + ((float)i / (float)light_count) * 2.0f * M_PI;

		float modifier = 0.25f * (cosf(2.5f * angle) + 2.0f);

		float x = cosf(angle) * radius + 0.33;
		float y = 1.0f + sinf(angle * 1.5f) * 0.125f;
		float z = sinf(angle) * radius;
		float w = modifier * 0.25f;
		p_app->obj.lights[i].data[3] = modifier;

		vec4 rotated_pos = { x, y, z, w };
		glm_vec4_copy(rotated_pos, ubo.lights[i].pos);
		glm_vec4_copy(rotated_pos, p_app->obj.lights[i].pos);
		glm_vec4_copy(p_app->obj.lights[i].data, ubo.lights[i].data);
		glm_vec4_copy(p_app->obj.lights[i].flags, ubo.lights[i].flags);
	}

	glm_vec4_copy(p_app->lighting.ambient, ubo.ambient_light);
	ubo.light_count = p_app->obj.light_count;

	memcpy(p_app->uniform.buffers_mapped[current_image], &ubo, sizeof(ubo));
}

void update_view(_app *p_app, float time) {
	if (glfwGetKey(p_app->win.window, GLFW_KEY_ESCAPE) == GLFW_PRESS && p_app->view.mouse_locked) {
		glfwSetInputMode(p_app->win.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		p_app->view.mouse_locked = false;
		p_app->view.first_mouse = true;
	}

	if (!p_app->view.mouse_locked) return;

	vec3 front;
	front[0] = cos(glm_rad(p_app->view.yaw)) * cos(glm_rad(p_app->view.pitch));
	front[1] = sin(glm_rad(p_app->view.pitch));
	front[2] = sin(glm_rad(p_app->view.yaw)) * cos(glm_rad(p_app->view.pitch));
	glm_vec3_normalize_to(front, front);

	vec3 world_up;
	glm_vec3_copy(p_app->view.world_up, world_up);

	vec3 right;
	glm_vec3_cross(front, world_up, right);
	glm_vec3_normalize(right);

	vec3 up;
	glm_vec3_cross(right, front, up);
	glm_vec3_normalize(up);

	if (glfwGetKey(p_app->win.window, GLFW_KEY_W) == GLFW_PRESS)
		p_app->view.cam_offset_goal[1] = p_app->view.speed;
	if (glfwGetKey(p_app->win.window, GLFW_KEY_S) == GLFW_PRESS)
		p_app->view.cam_offset_goal[1] = -p_app->view.speed;
	if (glfwGetKey(p_app->win.window, GLFW_KEY_A) == GLFW_PRESS)
		p_app->view.cam_offset_goal[0] = -p_app->view.speed;
	if (glfwGetKey(p_app->win.window, GLFW_KEY_D) == GLFW_PRESS)
		p_app->view.cam_offset_goal[0] = p_app->view.speed;
	if (glfwGetKey(p_app->win.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		p_app->view.cam_offset_goal[2] = -p_app->view.speed;
	if (glfwGetKey(p_app->win.window, GLFW_KEY_SPACE) == GLFW_PRESS)
		p_app->view.cam_offset_goal[2] = p_app->view.speed;

	for (int i = 0; i < 3; ++i) {
		float diff = p_app->view.cam_offset_goal[i] - p_app->view.cam_offset[i];
		p_app->view.cam_offset[i] += diff * p_app->view.lerp_speed * time * 60.0f;
	}

	vec3 offset_world = {0.0f}, tmp;
	glm_vec3_scale(right, p_app->view.cam_offset[0], tmp);
	glm_vec3_add(offset_world, tmp, offset_world);
	glm_vec3_scale(front, p_app->view.cam_offset[1], tmp);
	glm_vec3_add(offset_world, tmp, offset_world);
	glm_vec3_scale(world_up, p_app->view.cam_offset[2], tmp);
	glm_vec3_add(offset_world, tmp, offset_world);

	vec3 cam_actual_pos;
	glm_vec3_add(p_app->view.camera_pos, offset_world, cam_actual_pos);
	glm_vec3_copy(cam_actual_pos, p_app->view.camera_pos);
	glm_vec3_add(p_app->view.camera_pos, front, p_app->view.target);

	glm_vec3_zero(p_app->view.cam_offset_goal);
}
