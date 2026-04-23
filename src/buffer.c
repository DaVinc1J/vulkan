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
	if (p_app->obj.billboard_count > 0) {
		u32 count = p_app->obj.billboard_count;
		VkDeviceSize buffer_size = sizeof(_billboard) * count;

		if (p_app->billboard.instance_buffer != VK_NULL_HANDLE) {
			vmaDestroyBuffer(p_app->mem.alloc, p_app->billboard.instance_buffer, 
										p_app->billboard.instance_allocation);
		}

		VkBuffer staging_buffer;
		VmaAllocation staging_alloc;
		create_buffer(p_app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
								VMA_MEMORY_USAGE_CPU_ONLY, &staging_buffer, &staging_alloc);

		void* data;
		vmaMapMemory(p_app->mem.alloc, staging_alloc, &data);
		memcpy(data, p_app->obj.billboards, buffer_size);
		vmaUnmapMemory(p_app->mem.alloc, staging_alloc);

		create_buffer(p_app, buffer_size,
								VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
								VMA_MEMORY_USAGE_GPU_ONLY,
								&p_app->billboard.instance_buffer,
								&p_app->billboard.instance_allocation);

		copy_buffer(p_app, staging_buffer, p_app->billboard.instance_buffer, buffer_size);
		vmaDestroyBuffer(p_app->mem.alloc, staging_buffer, staging_alloc);

		p_app->billboard.current_buffer_size = buffer_size;
	}
}

void create_mesh_buffer(_app *p_app) {
	for (u32 i = 0; i < MESH_SHAPE_COUNT; i++) {

		VkBuffer*     vertex_buffer_ptr     = &p_app->mesh.vertex_buffers[i];
		VkBuffer*     index_buffer_ptr      = &p_app->mesh.index_buffers[i];
		VmaAllocation* vertex_alloc_ptr     = &p_app->mesh.vertex_allocations[i];
		VmaAllocation* index_alloc_ptr      = &p_app->mesh.index_allocations[i];

		u32 vertex_count = p_app->mesh.vertex_counts[i];
		u32 index_count  = p_app->mesh.index_counts[i];

		_vertex* vertices = p_app->mesh.vertices[i];
		u32*     indices  = p_app->mesh.indices[i];

		if (!vertex_count || !index_count || !vertices || !indices)
			continue;

		VkDeviceSize v_size = sizeof(_vertex) * vertex_count;
		VkDeviceSize i_size = sizeof(u32) * index_count;

		VkBuffer staging_vb;
		VmaAllocation staging_va;
		create_buffer(
			p_app,
			v_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,
			&staging_vb,
			&staging_va
		);

		void* v_data;
		vmaMapMemory(p_app->mem.alloc, staging_va, &v_data);
		memcpy(v_data, vertices, v_size);
		vmaUnmapMemory(p_app->mem.alloc, staging_va);

		create_buffer(
			p_app,
			v_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			vertex_buffer_ptr,
			vertex_alloc_ptr
		);

		copy_buffer(p_app, staging_vb, *vertex_buffer_ptr, v_size);
		vmaDestroyBuffer(p_app->mem.alloc, staging_vb, staging_va);

		VkBuffer staging_ib;
		VmaAllocation staging_ia;
		create_buffer(
			p_app,
			i_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,
			&staging_ib,
			&staging_ia
		);

		void* i_data;
		vmaMapMemory(p_app->mem.alloc, staging_ia, &i_data);
		memcpy(i_data, indices, i_size);
		vmaUnmapMemory(p_app->mem.alloc, staging_ia);

		create_buffer(
			p_app,
			i_size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			index_buffer_ptr,
			index_alloc_ptr
		);

		copy_buffer(p_app, staging_ib, *index_buffer_ptr, i_size);
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
			exit(EXIT_FAILURE);
		}

		p_app->uniform.buffers_mapped[i] = allocation_info.pMappedData;
	}
}

void create_storage_buffers(_app *p_app) {
	VkDeviceSize billboard_buffer_size = SBO_HEADER_SIZE + p_app->obj.billboard_count    * sizeof(_billboard);
	VkDeviceSize solar_object_buffer_size  = SBO_HEADER_SIZE + p_app->obj.solar_object_count * sizeof(_solar_object);

	p_app->storage.billboard_current_buffer_size = billboard_buffer_size;
	p_app->storage.solar_object_current_buffer_size = solar_object_buffer_size;

	p_app->storage.billboard_buffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
	p_app->storage.billboard_buffer_allocations = malloc(sizeof(VmaAllocation) * MAX_FRAMES_IN_FLIGHT);
	p_app->storage.billboard_buffers_mapped = malloc(sizeof(void*) * MAX_FRAMES_IN_FLIGHT);

	p_app->storage.solar_object_buffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
	p_app->storage.solar_object_buffer_allocations = malloc(sizeof(VmaAllocation) * MAX_FRAMES_IN_FLIGHT);
	p_app->storage.solar_object_buffers_mapped = malloc(sizeof(void*) * MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkBufferCreateInfo billboard_buffer_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = billboard_buffer_size,
			.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VkBufferCreateInfo solar_object_buffer_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = solar_object_buffer_size,
			.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VmaAllocationCreateInfo alloc_create_info = {
			.usage = VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};

		VmaAllocationInfo billboard_allocation_info;
		if (vmaCreateBuffer(p_app->mem.alloc, &billboard_buffer_create_info, &alloc_create_info,
											&p_app->storage.billboard_buffers[i],
											&p_app->storage.billboard_buffer_allocations[i],
											&billboard_allocation_info) != VK_SUCCESS) {

			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"storage buffers => failed to create storage buffers"
			);
			exit(EXIT_FAILURE);
		}

		p_app->storage.billboard_buffers_mapped[i] = billboard_allocation_info.pMappedData;

		VmaAllocationInfo solar_object_allocation_info;
		if (vmaCreateBuffer(p_app->mem.alloc, &solar_object_buffer_create_info, &alloc_create_info,
											&p_app->storage.solar_object_buffers[i],
											&p_app->storage.solar_object_buffer_allocations[i],
											&solar_object_allocation_info) != VK_SUCCESS) {

			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"storage buffers => failed to create storage buffers"
			);
			exit(EXIT_FAILURE);
		}

		p_app->storage.solar_object_buffers_mapped[i] = solar_object_allocation_info.pMappedData;
	}
}

void recreate_billboard_storage_buffers(_app *p_app, VkDeviceSize new_size) {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (p_app->storage.billboard_buffers[i] != VK_NULL_HANDLE) {
			vmaDestroyBuffer(p_app->mem.alloc, p_app->storage.billboard_buffers[i], p_app->storage.billboard_buffer_allocations[i]);
			p_app->storage.billboard_buffers[i] = VK_NULL_HANDLE;
			p_app->storage.billboard_buffer_allocations[i] = VK_NULL_HANDLE;
			p_app->storage.billboard_buffers_mapped[i] = NULL;
		}
	}

	p_app->storage.billboard_current_buffer_size = new_size;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkBufferCreateInfo buffer_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = new_size,
			.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VmaAllocationCreateInfo alloc_create_info = {
			.usage = VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};

		VmaAllocationInfo allocation_info;
		if (vmaCreateBuffer(p_app->mem.alloc, &buffer_create_info, &alloc_create_info,
											&p_app->storage.billboard_buffers[i],
											&p_app->storage.billboard_buffer_allocations[i],
											&allocation_info) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"storage buffers => failed to recreate storage buffers"
			);
			exit(EXIT_FAILURE);
		}

		p_app->storage.billboard_buffers_mapped[i] = allocation_info.pMappedData;
	}

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo sbo_info = {
			.buffer = p_app->storage.billboard_buffers[i],
			.offset = 0,
			.range = new_size,
		};

		VkWriteDescriptorSet descriptor_write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = p_app->descriptor.sets[i],
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo = &sbo_info,
		};

		vkUpdateDescriptorSets(p_app->device.logical, 1, &descriptor_write, 0, NULL);
	}
}

void recreate_solar_object_storage_buffers(_app *p_app, VkDeviceSize new_size) {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (p_app->storage.solar_object_buffers[i] != VK_NULL_HANDLE) {
			vmaDestroyBuffer(p_app->mem.alloc, p_app->storage.solar_object_buffers[i], p_app->storage.solar_object_buffer_allocations[i]);
			p_app->storage.solar_object_buffers[i] = VK_NULL_HANDLE;
			p_app->storage.solar_object_buffer_allocations[i] = VK_NULL_HANDLE;
			p_app->storage.solar_object_buffers_mapped[i] = NULL;
		}
	}

	p_app->storage.solar_object_current_buffer_size = new_size;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkBufferCreateInfo buffer_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = new_size,
			.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VmaAllocationCreateInfo alloc_create_info = {
			.usage = VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
		};

		VmaAllocationInfo allocation_info;
		if (vmaCreateBuffer(p_app->mem.alloc, &buffer_create_info, &alloc_create_info,
											&p_app->storage.solar_object_buffers[i],
											&p_app->storage.solar_object_buffer_allocations[i],
											&allocation_info) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"storage buffers => failed to recreate storage buffers"
			);
			exit(EXIT_FAILURE);
		}

		p_app->storage.solar_object_buffers_mapped[i] = allocation_info.pMappedData;
	}

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo sbo_info = {
			.buffer = p_app->storage.solar_object_buffers[i],
			.offset = 0,
			.range = new_size,
		};

		VkWriteDescriptorSet descriptor_write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = p_app->descriptor.sets[i],
			.dstBinding = 2,
			.dstArrayElement = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.pBufferInfo = &sbo_info,
		};

		vkUpdateDescriptorSets(p_app->device.logical, 1, &descriptor_write, 0, NULL);
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

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "command buffer => failed to begin record");
		exit(EXIT_FAILURE);
	}

	VkClearValue clear_values[2] = {
		{ .color = {{0.0f, 0.0f, 0.0f, 1.0f}} },
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
		.minDepth = 0.0f, .maxDepth = 1.0f
	};

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor = { .offset = {0, 0}, .extent = p_app->swp.extent };
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
												 p_app->pipeline.layout, 0, 1,
												 &p_app->descriptor.sets[p_app->sync.frame_index], 0, NULL);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.opaque);

	VkDeviceSize offset = 0;

	for (u32 i = 0; i < p_app->obj.solar_object_count; i++) {
		_solar_object *obj = &p_app->obj.solar_objects[i];

		if (obj->type == SOLAR_OBJECT_TYPE_LIGHT_EMIT) {
			continue;
		}

		vec3 diff;
		glm_vec3_sub(obj->position, p_app->view.camera_pos, diff);
		float dist2 = glm_vec3_dot(diff, diff);

		u32 lod;

		if (dist2 < p_app->config.lod.MESH_SPHERE_LOD_DISTANCES[0]) {
			lod = MESH_SPHERE_LOD3;
		} else if (dist2 < p_app->config.lod.MESH_SPHERE_LOD_DISTANCES[1]) {
			lod = MESH_SPHERE_LOD2;
		} else if (dist2 < p_app->config.lod.MESH_SPHERE_LOD_DISTANCES[2]) {
			lod = MESH_SPHERE_LOD1;
		} else {
			lod = MESH_SPHERE_LOD0;
		}

		vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_app->mesh.vertex_buffers[lod], &offset);
		vkCmdBindIndexBuffer(command_buffer, p_app->mesh.index_buffers[lod], 0, VK_INDEX_TYPE_UINT32);

		uint32_t object_index = i;
		vkCmdPushConstants(
			command_buffer,
			p_app->pipeline.layout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(_push_constants),
			&object_index
		);

		vkCmdDrawIndexed(command_buffer, p_app->mesh.index_counts[lod], 1, 0, 0, 0);
	}

	if (p_app->obj.billboard_count > 0) {
		_render_order* billboard_order = malloc(sizeof(_render_order) * p_app->obj.billboard_count);
		for (u32 i = 0; i < p_app->obj.billboard_count; ++i) {
			vec3 diff;
			glm_vec3_sub(p_app->view.camera_pos, p_app->obj.billboards[i].pos_w, diff);
			billboard_order[i] = (_render_order){ i, glm_vec3_dot(diff, diff) };
		}
		qsort(billboard_order, p_app->obj.billboard_count, sizeof(_render_order), compare_render_order);

		_billboard* sorted = malloc(sizeof(_billboard) * p_app->obj.billboard_count);
		for (u32 i = 0; i < p_app->obj.billboard_count; ++i)
			sorted[i] = p_app->obj.billboards[billboard_order[i].object_index];

		VkDeviceSize size = sizeof(_billboard) * p_app->obj.billboard_count;
		VkBuffer staging;
		VmaAllocation alloc;
		create_buffer(p_app, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &staging, &alloc);
		void* data;
		vmaMapMemory(p_app->mem.alloc, alloc, &data);
		memcpy(data, sorted, size);
		vmaUnmapMemory(p_app->mem.alloc, alloc);

		copy_buffer(p_app, staging, p_app->billboard.instance_buffer, size);
		vmaDestroyBuffer(p_app->mem.alloc, staging, alloc);

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.billboard);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_app->billboard.instance_buffer, &offset);
		vkCmdDraw(command_buffer, 6, p_app->obj.billboard_count, 0, 0);

		free(billboard_order);
		free(sorted);
	}

	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "command buffer => failed to end record");
		exit(EXIT_FAILURE);
	}
}

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
