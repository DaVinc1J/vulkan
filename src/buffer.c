#include "headers/buffer.h"
#include "headers/validation.h"

void create_buffer(_app *p_app, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VkBuffer *p_buffer, VmaAllocation *p_allocation) {
	VkBufferCreateInfo buffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo alloc_info = {
		.usage = memory_usage,
	};

	if (vmaCreateBuffer(p_app->mem.alloc, &buffer_create_info, &alloc_info, p_buffer, p_allocation, NULL) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"vma => failed to create buffer"
		);
		exit(EXIT_FAILURE);
	}
}

void copy_buffer(_app *p_app, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
	VkCommandBuffer command_buffer = begin_single_time_commands(p_app);

	VkBufferCopy copy_region = {
		.size = size,
	};
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

	end_single_time_commands(p_app, command_buffer);
}

void create_billboard_buffer(_app *p_app) {
	u32 count = p_app->obj.light_count;

	VkDeviceSize buffer_size = sizeof(_billboard) * count;

	VkBuffer staging_buffer;
	VmaAllocation staging_alloc;
	create_buffer(p_app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &staging_buffer, &staging_alloc);

	void* data;
	vmaMapMemory(p_app->mem.alloc, staging_alloc, &data);
	memcpy(data, p_app->obj.lights, buffer_size);
	vmaUnmapMemory(p_app->mem.alloc, staging_alloc);

	create_buffer(p_app,
							 buffer_size,
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							 VMA_MEMORY_USAGE_GPU_ONLY,
							 &p_app->billboard.instance_buffer,
							 &p_app->billboard.instance_allocation);

	copy_buffer(p_app, staging_buffer, p_app->billboard.instance_buffer, buffer_size);
	vmaDestroyBuffer(p_app->mem.alloc, staging_buffer, staging_alloc);
}

void create_mesh_buffer(_app *p_app) {
	u32 object_count = p_app->obj.object_count;

	p_app->mesh.vertex_buffers = malloc(sizeof(VkBuffer) * object_count);
	p_app->mesh.index_buffers = malloc(sizeof(VkBuffer) * object_count);
	p_app->mesh.vertex_allocations = malloc(sizeof(VmaAllocation) * object_count);
	p_app->mesh.index_allocations = malloc(sizeof(VmaAllocation) * object_count);

	for (u32 o = 0; o < object_count; ++o) {
		u32 v_count = p_app->obj.vertices_count[o];
		u32 i_count = p_app->obj.indices_count[o];

		VkDeviceSize v_size = sizeof(_vertex) * v_count;
		VkDeviceSize i_size = sizeof(u32) * i_count;

		VkBuffer staging_vb;
		VmaAllocation staging_va;
		create_buffer(p_app, v_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &staging_vb, &staging_va);
		void* v_data;
		vmaMapMemory(p_app->mem.alloc, staging_va, &v_data);
		memcpy(v_data, p_app->obj.vertices[o], v_size);
		vmaUnmapMemory(p_app->mem.alloc, staging_va);

		create_buffer(p_app, v_size,
								VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
								VMA_MEMORY_USAGE_GPU_ONLY,
								&p_app->mesh.vertex_buffers[o], &p_app->mesh.vertex_allocations[o]);

		copy_buffer(p_app, staging_vb, p_app->mesh.vertex_buffers[o], v_size);
		vmaDestroyBuffer(p_app->mem.alloc, staging_vb, staging_va);

		VkBuffer staging_ib;
		VmaAllocation staging_ia;
		create_buffer(p_app, i_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &staging_ib, &staging_ia);
		void* i_data;
		vmaMapMemory(p_app->mem.alloc, staging_ia, &i_data);
		memcpy(i_data, p_app->obj.indices[o], i_size);
		vmaUnmapMemory(p_app->mem.alloc, staging_ia);

		create_buffer(p_app, i_size,
								VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
								VMA_MEMORY_USAGE_GPU_ONLY,
								&p_app->mesh.index_buffers[o], &p_app->mesh.index_allocations[o]);

		copy_buffer(p_app, staging_ib, p_app->mesh.index_buffers[o], i_size);
		vmaDestroyBuffer(p_app->mem.alloc, staging_ib, staging_ia);
	}
}


void create_uniform_buffers(_app *p_app) {
	VkDeviceSize buffer_size = sizeof(_ubo);

	p_app->uniform.buffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
	p_app->uniform.buffer_allocations = malloc(sizeof(VmaAllocation) * MAX_FRAMES_IN_FLIGHT);
	p_app->uniform.buffers_mapped = malloc(sizeof(void*) * MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkBufferCreateInfo buffer_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = buffer_size,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VmaAllocationCreateInfo alloc_create_info = {
			.usage = VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};

		VmaAllocationInfo allocation_info;
		if (vmaCreateBuffer(p_app->mem.alloc, &buffer_create_info, &alloc_create_info,
											&p_app->uniform.buffers[i],
											&p_app->uniform.buffer_allocations[i],
											&allocation_info) != VK_SUCCESS) {

			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"uniform buffers => failed to create uniforms buffers"
			);
			exit(1);
			exit(EXIT_FAILURE);
		}

		p_app->uniform.buffers_mapped[i] = allocation_info.pMappedData;
	}
}

void create_command_pool(_app *p_app) {
	VkCommandPoolCreateInfo command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = p_app->device.queue_indices.graphics_family,
	};

	if (vkCreateCommandPool(p_app->device.logical, &command_pool_create_info, NULL, &p_app->cmd.pool) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"command pool => failed to create command pool"
		);
		exit(EXIT_FAILURE);
	}
}

void create_command_buffers(_app *p_app) {

	p_app->cmd.buffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo command_buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = p_app->cmd.pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
	};

	if (vkAllocateCommandBuffers(p_app->device.logical, &command_buffer_alloc_info, p_app->cmd.buffers) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"command buffer => failed to allocate command buffer"
		);
		exit(EXIT_FAILURE);
	}
}

int compare_render_order(const void* a, const void* b) {
	const _render_order* ro_a = (const _render_order*)a;
	const _render_order* ro_b = (const _render_order*)b;

	if (ro_a->distance < ro_b->distance) return 1;
	if (ro_a->distance > ro_b->distance) return -1;
	return 0;
}

void record_command_buffer(_app *p_app, VkCommandBuffer command_buffer, uint32_t image_index) {
	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
											 "command buffer => failed to begin record");
		exit(EXIT_FAILURE);
	}

	VkClearValue clear_values[2] = {
		{ .color = { {0.0f, 0.0f, 0.0f, 1.0f} } },
		{ .depthStencil = {1.0f, 0} }
	};

	VkRenderPassBeginInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = p_app->pipeline.render_pass,
		.framebuffer = p_app->pipeline.swapchain_framebuffers[image_index],
		.renderArea = { .offset = {0, 0}, .extent = p_app->swp.extent },
		.clearValueCount = 2,
		.pClearValues = clear_values,
	};

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = {
		.x = 0.0f, .y = 0.0f,
		.width = (float)p_app->swp.extent.width,
		.height = (float)p_app->swp.extent.height,
		.minDepth = 0.0f, .maxDepth = 1.0f,
	};
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor = { .offset = {0, 0}, .extent = p_app->swp.extent };
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
												 p_app->pipeline.layout, 0, 1,
												 &p_app->descriptor.sets[p_app->sync.frame_index], 0, NULL);

	_push_constants pc;
	glm_mat4_identity(pc.model);
	glm_scale(pc.model, (vec3){3.0f, 3.0f, 3.0f});

	mat3 normal3;
	glm_mat4_pick3(pc.model, normal3);
	glm_mat3_inv(normal3, normal3);
	glm_mat3_transpose(normal3);

	glm_mat4_identity(pc.normal);
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			pc.normal[i][j] = normal3[i][j];
		}
	}

	for (u32 o = 0; o < p_app->obj.object_count; ++o) {
		bool transparent = false;
		for (u32 k = 0; k < p_app->obj.indices_count[o]; ++k) {
			u32 tex_idx = p_app->obj.texture_indices[o][k];
			if (tex_idx != UINT32_MAX && p_app->tex.has_alpha[tex_idx]) {
				transparent = true;
				break;
			}
		}
		if (transparent) continue;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.opaque);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_app->mesh.vertex_buffers[o], &offset);
		vkCmdBindIndexBuffer(command_buffer, p_app->mesh.index_buffers[o], 0, VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(
			command_buffer,
			p_app->pipeline.layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(_push_constants),
			&pc
		);

		vkCmdDrawIndexed(command_buffer, p_app->obj.indices_count[o], 1, 0, 0, 0);
	}

	_render_order* transparent_draw_order = malloc(sizeof(_render_order) * p_app->obj.object_count);
	u32 t_count = 0;

	for (u32 o = 0; o < p_app->obj.object_count; ++o) {
		bool transparent = false;
		for (u32 k = 0; k < p_app->obj.indices_count[o]; ++k) {
			u32 tex_idx = p_app->obj.texture_indices[o][k];
			if (tex_idx != UINT32_MAX && p_app->tex.has_alpha[tex_idx]) {
				transparent = true;
				break;
			}
		}
		if (!transparent) continue;

		vec3 avg_pos = {0};
		u32 v_count = p_app->obj.vertices_count[o];
		for (u32 v = 0; v < v_count; ++v) {
			glm_vec3_add(avg_pos, p_app->obj.vertices[o][v].pos, avg_pos);
		}
		glm_vec3_scale(avg_pos, 1.0f / v_count, avg_pos);

		vec3 diff;
		glm_vec3_sub(p_app->view.camera_pos, avg_pos, diff);
		float dist_sq = glm_vec3_dot(diff, diff);

		transparent_draw_order[t_count++] = (_render_order){ o, dist_sq };
	}

	qsort(transparent_draw_order, t_count, sizeof(_render_order), compare_render_order);

	for (u32 i = 0; i < t_count; ++i) {
		u32 o = transparent_draw_order[i].object_index;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.transparent);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_app->mesh.vertex_buffers[o], &offset);
		vkCmdBindIndexBuffer(command_buffer, p_app->mesh.index_buffers[o], 0, VK_INDEX_TYPE_UINT32);


		vkCmdPushConstants(
			command_buffer,
			p_app->pipeline.layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(_push_constants),
			&pc
		);

		vkCmdDrawIndexed(command_buffer, p_app->obj.indices_count[o], 1, 0, 0, 0);
	}

	free(transparent_draw_order);


	u32 light_count = p_app->obj.light_count;
	if (light_count > 0) {

		_render_order* light_order = malloc(sizeof(_render_order) * light_count);
		for (u32 i = 0; i < light_count; ++i) {
			vec3 diff;
			glm_vec3_sub(p_app->view.camera_pos, p_app->obj.lights[i].pos, diff);
			float dist_sq = glm_vec3_dot(diff, diff);
			light_order[i] = (_render_order){ i, dist_sq };
		}
		qsort(light_order, light_count, sizeof(_render_order), compare_render_order);

		_billboard* sorted_instances = malloc(sizeof(_billboard) * light_count);
		for (u32 i = 0; i < light_count; ++i) {
			u32 idx = light_order[i].object_index;
			_billboard* src = &p_app->obj.lights[idx];

			glm_vec4_copy(src->pos,sorted_instances[i].pos);
			glm_vec4_copy(src->data, sorted_instances[i].data);
			glm_vec4_copy(src->flags, sorted_instances[i].flags);
		}

		VkDeviceSize buffer_size = sizeof(_billboard) * light_count;

		VkBuffer staging_buffer;
		VmaAllocation staging_alloc;
		create_buffer(p_app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &staging_buffer, &staging_alloc);

		void* data;
		vmaMapMemory(p_app->mem.alloc, staging_alloc, &data);
		memcpy(data, sorted_instances, buffer_size);
		vmaUnmapMemory(p_app->mem.alloc, staging_alloc);

		copy_buffer(p_app, staging_buffer, p_app->billboard.instance_buffer, buffer_size);
		vmaDestroyBuffer(p_app->mem.alloc, staging_buffer, staging_alloc);

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.billboard);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_app->billboard.instance_buffer, &offset);

		vkCmdDraw(command_buffer, 6, light_count, 0, 0);

		free(light_order);
		free(sorted_instances);
	}

	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
											 "command buffer => failed to end record");
		exit(EXIT_FAILURE);
	}
}

//// begin single time command buffer ////
VkCommandBuffer begin_single_time_commands(_app *p_app) {
	VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = p_app->cmd.pool,
		.commandBufferCount = 1,
	};

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(p_app->device.logical, &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;
}

//// end single time command buffer ////
void end_single_time_commands(_app *p_app, VkCommandBuffer command_buffer) {
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
	};

	vkQueueSubmit(p_app->device.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(p_app->device.graphics_queue);

	vkFreeCommandBuffers(p_app->device.logical, p_app->cmd.pool, 1, &command_buffer);
}
