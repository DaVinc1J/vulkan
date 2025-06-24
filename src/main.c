#define FAST_OBJ_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
#include "define.h"
#include "app.h"
#include "window.h"
#include "vk_validation.h"
#include "object.h"
#include "vk_core.h"
#include "vk_swapchain.h"
#include "vk_renderpass.h"
#include "vk_pipeline.h"
#include "vk_sync.h"
#include "vk_buffer.h"
#include "vk_image.h"
#include "vk_descriptors.h"
#include "vk_loop.h"

void vulkan_init(_app *p_app);
void clean(_app *p_app);
void main_loop(_app *p_app);

const u32 MAX_FRAMES_IN_FLIGHT = 2;

u32 clamp(u32 n, u32 min, u32 max) {
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

int main() {
	_app app = {0};
	app_init(&app);
	window_init(&app);
	vulkan_init(&app);
	main_loop(&app);
	clean(&app);
}

void vulkan_init(_app *p_app) {
	read_obj_file(p_app);
	create_instance(p_app);
	setup_debug_messenger(p_app);
	create_surface(p_app);
	pick_physical_device(p_app);
	create_logical_device(p_app);
	create_alloc(p_app);
	create_swapchain(p_app);
	create_image_views(p_app);
	create_render_pass(p_app);
	create_descriptor_set_layout(p_app);
	create_graphics_pipelines(p_app);
	create_command_pool(p_app);
	create_colour_resources(p_app);
	create_depth_resources(p_app);
	create_framebuffers(p_app);
	create_texture_image(p_app);
	create_texture_image_view(p_app);
	create_texture_sampler(p_app);
	create_billboard_buffer(p_app);
	create_mesh_buffer(p_app);
	create_uniform_buffers(p_app);
	create_descriptor_pool(p_app);
	create_descriptor_sets(p_app);
	create_command_buffers(p_app);
	create_sync_objects(p_app);
}

void main_loop(_app *p_app) {

	while (!glfwWindowShouldClose(p_app->win.window)) {
		glfwPollEvents();
		log_performance(p_app);
		update_view(p_app, get_delta_time());
		draw_frame(p_app);
	}

	vkDeviceWaitIdle(p_app->device.logical);
}

void clean(_app *p_app) {
	for (u32 o = 0; o < p_app->obj.object_count; ++o) {
		free(p_app->obj.vertices[o]);
		free(p_app->obj.indices[o]);
		free(p_app->obj.face_indices[o]);
		free(p_app->obj.material_indices[o]);
		free(p_app->obj.group_indices[o]);
		free(p_app->obj.object_indices[o]);
		free(p_app->obj.texture_indices[o]);
	}
	free(p_app->obj.vertices);
	free(p_app->obj.indices);
	free(p_app->obj.face_indices);
	free(p_app->obj.material_indices);
	free(p_app->obj.group_indices);
	free(p_app->obj.object_indices);
	free(p_app->obj.texture_indices);
	free(p_app->obj.vertices_count);
	free(p_app->obj.indices_count);

	p_app->obj.vertices = NULL;
	p_app->obj.indices = NULL;
	p_app->obj.face_indices = NULL;
	p_app->obj.material_indices = NULL;
	p_app->obj.group_indices = NULL;
	p_app->obj.object_indices = NULL;
	p_app->obj.texture_indices = NULL;
	p_app->obj.vertices_count = NULL;
	p_app->obj.indices_count = NULL;

	for (u32 i = 0; i < p_app->obj.texture_count; i++) {
		free(p_app->obj.textures[i].path);
		free(p_app->obj.textures[i].name);
	}
	free(p_app->obj.textures);
	p_app->obj.textures = NULL;

	vkDestroyImageView(p_app->device.logical, p_app->depth.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->depth.image, p_app->depth.image_allocation);
	vkDestroyImageView(p_app->device.logical, p_app->colour.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->colour.image, p_app->colour.image_allocation);

	vkDestroySampler(p_app->device.logical, p_app->tex.sampler, NULL);
	for (u32 i = 0; i < p_app->obj.texture_count; i++) {
		if (p_app->tex.image_views[i]) {
			vkDestroyImageView(p_app->device.logical, p_app->tex.image_views[i], NULL);
		}
		if (p_app->tex.images[i]) {
			vmaDestroyImage(p_app->mem.alloc, p_app->tex.images[i], p_app->tex.image_allocations[i]);
		}
	}
	free(p_app->tex.image_views);
	free(p_app->tex.images);
	free(p_app->tex.image_allocations);
	free(p_app->tex.has_alpha);
	free(p_app->tex.mip_levels);
	p_app->tex.image_views = NULL;
	p_app->tex.images = NULL;
	p_app->tex.image_allocations = NULL;
	p_app->tex.has_alpha = NULL;
	p_app->tex.mip_levels = NULL;

	vkDestroyDescriptorPool(p_app->device.logical, p_app->descriptor.pool, NULL);
	vkDestroyDescriptorSetLayout(p_app->device.logical, p_app->pipeline.descriptor_set_layout, NULL);
	free(p_app->descriptor.sets);
	p_app->descriptor.sets = NULL;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vmaDestroyBuffer(p_app->mem.alloc, p_app->uniform.buffers[i], p_app->uniform.buffer_allocations[i]);
	}
	free(p_app->uniform.buffers);
	free(p_app->uniform.buffer_allocations);
	free(p_app->uniform.buffers_mapped);
	p_app->uniform.buffers = NULL;
	p_app->uniform.buffer_allocations = NULL;
	p_app->uniform.buffers_mapped = NULL;


	vmaDestroyBuffer(p_app->mem.alloc, p_app->billboard.instance_buffer, p_app->billboard.instance_allocation);

	for (u32 i = 0; i < p_app->obj.object_count; i++) {
		vmaDestroyBuffer(p_app->mem.alloc, p_app->mesh.vertex_buffers[i], p_app->mesh.vertex_allocations[i]);
		vmaDestroyBuffer(p_app->mem.alloc, p_app->mesh.index_buffers[i], p_app->mesh.index_allocations[i]);
	}
	free(p_app->mesh.vertex_buffers);
	free(p_app->mesh.index_buffers);
	free(p_app->mesh.vertex_allocations);
	free(p_app->mesh.index_allocations);
	p_app->mesh.vertex_buffers = NULL;
	p_app->mesh.index_buffers = NULL;
	p_app->mesh.vertex_allocations = NULL;
	p_app->mesh.index_allocations = NULL;

	free(p_app->cmd.buffers);
	p_app->cmd.buffers = NULL;

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroySemaphore(p_app->device.logical, p_app->sync.image_available_semaphores[i], NULL);
		vkDestroySemaphore(p_app->device.logical, p_app->sync.render_finished_semaphores[i], NULL);
	}
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(p_app->device.logical, p_app->sync.in_flight_fences[i], NULL);
	}
	free(p_app->sync.image_available_semaphores);
	free(p_app->sync.render_finished_semaphores);
	free(p_app->sync.in_flight_fences);
	p_app->sync.image_available_semaphores = NULL;
	p_app->sync.render_finished_semaphores = NULL;
	p_app->sync.in_flight_fences = NULL;

	vkDestroyCommandPool(p_app->device.logical, p_app->cmd.pool, NULL);
	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroyFramebuffer(p_app->device.logical, p_app->pipeline.swapchain_framebuffers[i], NULL);
	}
	free(p_app->pipeline.swapchain_framebuffers);
	p_app->pipeline.swapchain_framebuffers = NULL;

	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.opaque, NULL);
	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.transparent, NULL);
	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.billboard, NULL);
	vkDestroyPipelineLayout(p_app->device.logical, p_app->pipeline.layout, NULL);
	vkDestroyRenderPass(p_app->device.logical, p_app->pipeline.render_pass, NULL);

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroyImageView(p_app->device.logical, p_app->swp.image_views[i], NULL);
	}
	free(p_app->swp.image_views);
	p_app->swp.image_views = NULL;
	vkDestroySwapchainKHR(p_app->device.logical, p_app->swp.swapchain, NULL);

	vmaDestroyAllocator(p_app->mem.alloc);
	vkDestroyDevice(p_app->device.logical, NULL);

	if (enable_validation_layers) {
		destroy_debug_utils_messenger_EXT(p_app->inst.instance, p_app->inst.debug_messenger, NULL);
	}

	vkDestroySurfaceKHR(p_app->inst.instance, p_app->swp.surface, NULL);
	vkDestroyInstance(p_app->inst.instance, NULL);
	glfwDestroyWindow(p_app->win.window);
	glfwTerminate();
}
