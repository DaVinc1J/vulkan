#include "vk_descriptors.h"
#include "vk_validation.h"

void create_descriptor_set_layout(_app *p_app) {
	VkDescriptorSetLayoutBinding ubo_layout_binding = {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = NULL,
	};

	VkDescriptorSetLayoutBinding sampler_layout_binding = {
		.binding = 1,
		.descriptorCount = p_app->obj.texture_count,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImmutableSamplers = NULL,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};

	VkDescriptorSetLayoutBinding bindings[] = {
		ubo_layout_binding,
		sampler_layout_binding,
	};

	VkDescriptorSetLayoutCreateInfo ubo_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = sizeof(bindings) / sizeof(bindings[0]),
		.pBindings = bindings,
	};

	if (vkCreateDescriptorSetLayout(p_app->device.logical, &ubo_layout_create_info, NULL, &p_app->pipeline.descriptor_set_layout) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"descriptor set layout => failed to create descriptor set layout"
		);
		exit(EXIT_FAILURE);
	}
}

void create_descriptor_pool(_app *p_app) {
	VkDescriptorPoolSize pool_sizes[2] = {
		{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = MAX_FRAMES_IN_FLIGHT },
		{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = MAX_FRAMES_IN_FLIGHT * p_app->obj.texture_count }
	};

	VkDescriptorPoolCreateInfo pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = 2,
		.pPoolSizes = pool_sizes,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
	};

	if (vkCreateDescriptorPool(p_app->device.logical, &pool_create_info, NULL, &p_app->descriptor.pool) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"descriptor pool => failed to create descriptor pool"
		);
		exit(EXIT_FAILURE);
	}
}

void create_descriptor_sets(_app *p_app) {
	p_app->descriptor.sets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);
	VkDescriptorSetLayout *layouts = malloc(sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		layouts[i] = p_app->pipeline.descriptor_set_layout;
	}

	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = p_app->descriptor.pool,
		.descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
		.pSetLayouts = layouts,
	};

	if (vkAllocateDescriptorSets(p_app->device.logical, &alloc_info, p_app->descriptor.sets) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"descriptor sets => failed to allocate descriptor sets"
		);
		exit(EXIT_FAILURE);
	}

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo buffer_info = {
			.buffer = p_app->uniform.buffers[i],
			.offset = 0,
			.range = sizeof(_ubo)
		};

		u32 valid_tex_count = 0;
		for (u32 j = 0; j < p_app->obj.texture_count; j++) {
			if (p_app->tex.image_views[j] != VK_NULL_HANDLE) {
				valid_tex_count++;
			}
		}

		VkDescriptorImageInfo *image_infos = malloc(sizeof(VkDescriptorImageInfo) * valid_tex_count);
		u32 tex_i = 0;
		for (u32 j = 0; j < p_app->obj.texture_count; j++) {
			if (p_app->tex.image_views[j] == VK_NULL_HANDLE) continue;
			image_infos[tex_i++] = (VkDescriptorImageInfo){
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.imageView = p_app->tex.image_views[j],
				.sampler = p_app->tex.sampler,
			};
		}

		VkWriteDescriptorSet descriptor_writes[2] = {
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = p_app->descriptor.sets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = 1,
				.pBufferInfo = &buffer_info,
			},
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = p_app->descriptor.sets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = valid_tex_count,
				.pImageInfo = image_infos,
			}
		};

		vkUpdateDescriptorSets(p_app->device.logical, 2, descriptor_writes, 0, NULL);
		free(image_infos);
	}

	free(layouts);
}
