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
	u32 primitive_count = p_app->obj.primitive_count;
	p_app->mesh.vertex_buffers = malloc(sizeof(VkBuffer) * primitive_count);
	p_app->mesh.index_buffers = malloc(sizeof(VkBuffer) * primitive_count);
	p_app->mesh.vertex_allocations = malloc(sizeof(VmaAllocation) * primitive_count);
	p_app->mesh.index_allocations = malloc(sizeof(VmaAllocation) * primitive_count);
	p_app->mesh.vertex_count = malloc(sizeof(u32) * primitive_count);
	p_app->mesh.index_count = malloc(sizeof(u32) * primitive_count);
	p_app->mesh.is_transparent = malloc(sizeof(u8) * primitive_count);
	p_app->mesh.centroids = malloc(sizeof(vec3) * primitive_count);
	p_app->mesh.tex_index = malloc(sizeof(u32) * primitive_count);

	u32 prim_index = 0;
	for (u32 f = 0; f < p_app->tex.file.count; f++) {
		cgltf_data *data = p_app->obj.data[f];

		float x_min = FLT_MAX, x_max = -FLT_MAX;
		float y_min = FLT_MAX, y_max = -FLT_MAX;
		float z_min = FLT_MAX, z_max = -FLT_MAX;

		for (u32 m = 0; m < data->meshes_count; m++) {
			cgltf_mesh *mesh = &data->meshes[m];

			for (u32 p =0; p < mesh->primitives_count; p++, prim_index++) {
				cgltf_primitive* prim = &mesh->primitives[p];

				cgltf_accessor* pos_acc = NULL;
				cgltf_accessor* norm_acc = NULL;
				cgltf_accessor* tex_acc = NULL;

				for (size_t a = 0; a < prim->attributes_count; a++) {
					cgltf_attribute* attr = &prim->attributes[a];
					if (attr->type == cgltf_attribute_type_position) pos_acc = attr->data;
					if (attr->type == cgltf_attribute_type_normal) norm_acc = attr->data;
					if (attr->type == cgltf_attribute_type_texcoord && attr->index == 0) tex_acc = attr->data;
				}
				if (!pos_acc) continue;

				int tex_index = 0;
				if (prim->material && prim->material->pbr_metallic_roughness.base_color_texture.texture) {
					cgltf_texture* tex = prim->material->pbr_metallic_roughness.base_color_texture.texture;
					tex_index = (int)(tex - data->textures);
				}
				_texture tex = p_app->atlas.textures[tex_index];
				p_app->mesh.is_transparent[prim_index] = (tex.flags & TEXTURE_FLAG_HAS_ALPHA) != 0;
				p_app->mesh.tex_index[prim_index] = tex_index;

				u32 v_count = (u32)pos_acc->count;
				u32 i_count = prim->indices ? (u32)prim->indices->count : v_count;

				_vertex* vertices = malloc(sizeof(_vertex) * v_count);

				for (u32 i = 0; i < v_count; i++) {
					float pos[3];
					cgltf_accessor_read_float(pos_acc, i, pos, 3);
					if (pos[0] < x_min) {x_min = pos[0];}
					if (pos[0] > x_max) {x_max = pos[0];}
					if (pos[1] < y_min) {y_min = pos[1];}
					if (pos[1] > y_max) {y_max = pos[1];}
					if (pos[2] < z_min) {z_min = pos[2];}
					if (pos[2] > z_max) {z_max = pos[2];}
				}

				for (u32 i = 0; i < v_count; i++) {
					cgltf_accessor_read_float(pos_acc, i, vertices[i].pos, 3);
					vertices[i].pos[0] -= -1520.0f;
					vertices[i].pos[1] -= 3456.0f;
					vertices[i].pos[2] -= 64.0f;

					float y = vertices[i].pos[1];
					float z = vertices[i].pos[2];

					vertices[i].pos[1] = z;
					vertices[i].pos[2] = -y;

					printf("%.2f, %.2f, %.2f\n", vertices[i].pos[0], vertices[i].pos[1], vertices[i].pos[2]);

					if (norm_acc) cgltf_accessor_read_float(norm_acc, i, vertices[i].norm, 3);
					else memset(vertices[i].norm, 0, sizeof(float) * 3);

					if (tex_acc) {
						float uv[2];
						cgltf_accessor_read_float(tex_acc, i, uv, 2);
						float u = uv[0];
						float v = 1.0f - uv[1];

						if (tex.flags & TEXTURE_FLAG_IS_ROTATED) {
							float tmp = u;
							u = 1.0f - v;
							v = tmp;
						}

						vertices[i].tex[0] = u;
						vertices[i].tex[1] = v;
					}

					vertices[i].tex_index = 0;
				}

				u32* indices = malloc(sizeof(u32) * i_count);
				if (prim->indices) {
					for (u32 i = 0; i < i_count; ++i) {
						indices[i] = (u32)cgltf_accessor_read_index(prim->indices, i);
					}
				} else {
					for (u32 i = 0; i < v_count; ++i) indices[i] = i;
				}

				p_app->mesh.vertex_count[prim_index] = v_count;
				p_app->mesh.index_count[prim_index] = i_count;

				vec3 avg = {0};
				for (u32 v = 0; v < v_count; ++v) {
					avg[0] += vertices[v].pos[0];
					avg[1] += vertices[v].pos[1];
					avg[2] += vertices[v].pos[2];
				}
				avg[0] /= v_count;
				avg[1] /= v_count;
				avg[2] /= v_count;
				p_app->mesh.centroids[prim_index][0] = avg[0];
				p_app->mesh.centroids[prim_index][1] = avg[1];
				p_app->mesh.centroids[prim_index][2] = avg[2];

				VkDeviceSize v_size = sizeof(_vertex) * v_count;
				VkDeviceSize i_size = sizeof(u32) * i_count;

				VkBuffer staging_vb;
				VmaAllocation staging_va;
				create_buffer(p_app, v_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &staging_vb, &staging_va);
				void* v_data;
				vmaMapMemory(p_app->mem.alloc, staging_va, &v_data);
				memcpy(v_data, vertices, v_size);
				vmaUnmapMemory(p_app->mem.alloc, staging_va);

				create_buffer(p_app, v_size,
									VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
									VMA_MEMORY_USAGE_GPU_ONLY,
									&p_app->mesh.vertex_buffers[prim_index], &p_app->mesh.vertex_allocations[prim_index]);
				copy_buffer(p_app, staging_vb, p_app->mesh.vertex_buffers[prim_index], v_size);
				vmaDestroyBuffer(p_app->mem.alloc, staging_vb, staging_va);

				VkBuffer staging_ib;
				VmaAllocation staging_ia;
				create_buffer(p_app, i_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &staging_ib, &staging_ia);
				void* i_data;
				vmaMapMemory(p_app->mem.alloc, staging_ia, &i_data);
				memcpy(i_data, indices, i_size);
				vmaUnmapMemory(p_app->mem.alloc, staging_ia);

				create_buffer(p_app, i_size,
									VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
									VMA_MEMORY_USAGE_GPU_ONLY,
									&p_app->mesh.index_buffers[prim_index], &p_app->mesh.index_allocations[prim_index]);
				copy_buffer(p_app, staging_ib, p_app->mesh.index_buffers[prim_index], i_size);
				vmaDestroyBuffer(p_app->mem.alloc, staging_ib, staging_ia);

				free(vertices);
				free(indices);
			}
		}

		float x_avg = (x_max + x_min) / 2;
		float y_avg = (y_max + y_min) / 2;
		float z_avg = (z_max + z_min) / 2;

		printf("%.2f, %.2f, %.2f\n", x_avg, y_avg, z_avg);

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

	_render_order* sort_list = malloc(sizeof(_render_order) * p_app->obj.primitive_count);
	u32 count = 0;
	for (u32 i = 0; i < p_app->obj.primitive_count; ++i) {
		if (p_app->mesh.is_transparent[i]) {
			vec3 diff;
			glm_vec3_sub(p_app->view.camera_pos, p_app->mesh.centroids[i], diff);
			float dist_sq = glm_vec3_dot(diff, diff);
			sort_list[count++] = (_render_order){ .object_index = i, .distance = dist_sq };
		} else {
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.opaque);

			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_app->mesh.vertex_buffers[i], &offset);
			vkCmdBindIndexBuffer(command_buffer, p_app->mesh.index_buffers[i], 0, VK_INDEX_TYPE_UINT32);

			_push_constants pc = {0};
			float scale = (float)p_app->atlas.scale;
			u32 tex_index = p_app->mesh.tex_index[i];
			_texture tex = p_app->atlas.textures[tex_index];

			glm_mat4_identity(pc.model);
			glm_scale(pc.model, (vec3){0.01f, 0.01f, 0.01f});

			mat3 normal3;
			glm_mat4_pick3(pc.model, normal3);
			glm_mat3_inv(normal3, normal3);
			glm_mat3_transpose(normal3);
			glm_mat4_identity(pc.normal);
			for (int r = 0; r < 3; ++r) {
				for (int c = 0; c < 3; ++c) {
					pc.normal[r][c] = normal3[r][c];
				}
			}

			pc.offset[0] = (float)tex.x / (float)scale;
			pc.offset[1] = (float)tex.y / (float)scale;
			pc.scale[0] = (float)tex.w / (float)scale;
			pc.scale[1] = (float)tex.h / (float)scale;

			vkCmdPushConstants(command_buffer, p_app->pipeline.layout,
											VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
											0, sizeof(_push_constants), &pc);

			vkCmdDrawIndexed(command_buffer, p_app->mesh.index_count[i], 1, 0, 0, 0);
		}
	}

	qsort(sort_list, count, sizeof(_render_order), compare_render_order);

	for (u32 i = 0; i < count; ++i) {
		u32 prim = sort_list[i].object_index;
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.transparent);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_app->mesh.vertex_buffers[prim], &offset);
		vkCmdBindIndexBuffer(command_buffer, p_app->mesh.index_buffers[prim], 0, VK_INDEX_TYPE_UINT32);

		_push_constants pc = {0};
		float scale = (float)p_app->atlas.scale;
		u32 tex_index = p_app->mesh.tex_index[i];
		_texture tex = p_app->atlas.textures[tex_index];

		glm_mat4_identity(pc.model);
		glm_scale(pc.model, (vec3){0.01f, 0.01f, 0.01f});

		mat3 normal3;
		glm_mat4_pick3(pc.model, normal3);
		glm_mat3_inv(normal3, normal3);
		glm_mat3_transpose(normal3);
		glm_mat4_identity(pc.normal);
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 3; ++c) {
				pc.normal[r][c] = normal3[r][c];
			}
		}

		pc.offset[0] = (float)tex.x / (float)scale;
		pc.offset[1] = (float)tex.y / (float)scale;
		pc.scale[0] = (float)tex.w / (float)scale;
		pc.scale[1] = (float)tex.h / (float)scale;


		vkCmdPushConstants(command_buffer, p_app->pipeline.layout,
										 VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
										 0, sizeof(_push_constants), &pc);

		vkCmdDrawIndexed(command_buffer, p_app->mesh.index_count[prim], 1, 0, 0, 0);
	}

	free(sort_list);

	if (p_app->obj.light_count > 0) {
		_render_order* light_order = malloc(sizeof(_render_order) * p_app->obj.light_count);
		for (u32 i = 0; i < p_app->obj.light_count; ++i) {
			vec3 diff;
			glm_vec3_sub(p_app->view.camera_pos, p_app->obj.lights[i].pos, diff);
			light_order[i] = (_render_order){ i, glm_vec3_dot(diff, diff) };
		}
		qsort(light_order, p_app->obj.light_count, sizeof(_render_order), compare_render_order);

		_billboard* sorted = malloc(sizeof(_billboard) * p_app->obj.light_count);
		for (u32 i = 0; i < p_app->obj.light_count; ++i)
			sorted[i] = p_app->obj.lights[light_order[i].object_index];

		VkDeviceSize size = sizeof(_billboard) * p_app->obj.light_count;
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
		vkCmdDraw(command_buffer, 6, p_app->obj.light_count, 0, 0);

		free(light_order);
		free(sorted);
	}

	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "command buffer => failed to end record");
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
