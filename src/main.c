#define FAST_OBJ_IMPLEMENTATION
#define CGLTF_IMPLEMENTATION
#include "headers/define.h"
#include "headers/app.h"
#include "headers/window.h"
#include "headers/validation.h"
#include "headers/object.h"
#include "headers/core.h"
#include "headers/swapchain.h"
#include "headers/renderpass.h"
#include "headers/pipeline.h"
#include "headers/sync.h"
#include "headers/buffer.h"
#include "headers/image.h"
#include "headers/descriptors.h"
#include "headers/loop.h"

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
	load_gltf(p_app);
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
	vkDestroyImageView(p_app->device.logical, p_app->depth.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->depth.image, p_app->depth.image_allocation);
	vkDestroyImageView(p_app->device.logical, p_app->colour.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->colour.image, p_app->colour.image_allocation);

	vkDestroySampler(p_app->device.logical, p_app->tex.sampler, NULL);
	for (u32 i = 0; i < p_app->tex.atlas.count; i++) {
		if (p_app->tex.image_views[i]) {
			vkDestroyImageView(p_app->device.logical, p_app->tex.image_views[i], NULL);
		}
		if (p_app->tex.images[i]) {
			vmaDestroyImage(p_app->mem.alloc, p_app->tex.images[i], p_app->tex.image_allocations[i]);
		}
	}
	free(p_app->tex.image_views);           p_app->tex.image_views = NULL;
	free(p_app->tex.images);                p_app->tex.images = NULL;
	free(p_app->tex.image_allocations);     p_app->tex.image_allocations = NULL;
	free(p_app->tex.mip_levels);            p_app->tex.mip_levels = NULL;

	vkDestroyDescriptorPool(p_app->device.logical, p_app->descriptor.pool, NULL);
	vkDestroyDescriptorSetLayout(p_app->device.logical, p_app->pipeline.descriptor_set_layout, NULL);
	free(p_app->descriptor.sets);           p_app->descriptor.sets = NULL;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vmaDestroyBuffer(p_app->mem.alloc, p_app->uniform.buffers[i], p_app->uniform.buffer_allocations[i]);
	}
	free(p_app->uniform.buffers);           p_app->uniform.buffers = NULL;
	free(p_app->uniform.buffer_allocations);p_app->uniform.buffer_allocations = NULL;
	free(p_app->uniform.buffers_mapped);    p_app->uniform.buffers_mapped = NULL;

	vmaDestroyBuffer(p_app->mem.alloc, p_app->billboard.instance_buffer, p_app->billboard.instance_allocation);

	for (u32 i = 0; i < p_app->obj.primitive_count; i++) {
		vmaDestroyBuffer(p_app->mem.alloc, p_app->mesh.vertex_buffers[i], p_app->mesh.vertex_allocations[i]);
		vmaDestroyBuffer(p_app->mem.alloc, p_app->mesh.index_buffers[i], p_app->mesh.index_allocations[i]);
	}
	free(p_app->mesh.vertex_buffers);       p_app->mesh.vertex_buffers = NULL;
	free(p_app->mesh.index_buffers);        p_app->mesh.index_buffers = NULL;
	free(p_app->mesh.vertex_allocations);   p_app->mesh.vertex_allocations = NULL;
	free(p_app->mesh.index_allocations);    p_app->mesh.index_allocations = NULL;
	free(p_app->mesh.vertex_count);         p_app->mesh.vertex_count = NULL;
	free(p_app->mesh.index_count);          p_app->mesh.index_count = NULL;
	free(p_app->mesh.is_transparent);       p_app->mesh.is_transparent = NULL;
	free(p_app->mesh.centroids);						p_app->mesh.centroids = NULL;

	free(p_app->cmd.buffers);               p_app->cmd.buffers = NULL;

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroySemaphore(p_app->device.logical, p_app->sync.image_available_semaphores[i], NULL);
		vkDestroySemaphore(p_app->device.logical, p_app->sync.render_finished_semaphores[i], NULL);
	}
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(p_app->device.logical, p_app->sync.in_flight_fences[i], NULL);
	}
	free(p_app->sync.image_available_semaphores);  p_app->sync.image_available_semaphores = NULL;
	free(p_app->sync.render_finished_semaphores);  p_app->sync.render_finished_semaphores = NULL;
	free(p_app->sync.in_flight_fences);            p_app->sync.in_flight_fences = NULL;

	vkDestroyCommandPool(p_app->device.logical, p_app->cmd.pool, NULL);
	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroyFramebuffer(p_app->device.logical, p_app->pipeline.swapchain_framebuffers[i], NULL);
	}
	free(p_app->pipeline.swapchain_framebuffers); p_app->pipeline.swapchain_framebuffers = NULL;

	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.opaque, NULL);
	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.transparent, NULL);
	vkDestroyPipeline(p_app->device.logical, p_app->pipeline.billboard, NULL);
	vkDestroyPipelineLayout(p_app->device.logical, p_app->pipeline.layout, NULL);
	vkDestroyRenderPass(p_app->device.logical, p_app->pipeline.render_pass, NULL);

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		vkDestroyImageView(p_app->device.logical, p_app->swp.image_views[i], NULL);
	}
	free(p_app->swp.image_views);           p_app->swp.image_views = NULL;
	vkDestroySwapchainKHR(p_app->device.logical, p_app->swp.swapchain, NULL);

	for (u32 i = 0; i < p_app->tex.atlas.count; i++) {
		cgltf_free(p_app->obj.data[i]);
	}
	free(p_app->obj.data);                  p_app->obj.data = NULL;

	free(p_app->obj.lights);                p_app->obj.lights = NULL;

	for (u32 i = 0; i < p_app->tex.file.count; i++) {
		free(p_app->tex.file.names[i]);
	}
	free(p_app->tex.file.names);            p_app->tex.file.names = NULL;

	for (u32 i = 0; i < p_app->tex.atlas.count; i++) {
		free(p_app->tex.atlas.names[i]);
	}
	free(p_app->tex.atlas.names);           p_app->tex.atlas.names = NULL;

	free(p_app->atlas.textures);            p_app->atlas.textures = NULL;

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
