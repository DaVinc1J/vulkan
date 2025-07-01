#include "headers/image.h"
#include "headers/validation.h"
#include "headers/buffer.h"
#include "headers/swapchain.h"

VkFormat find_supported_format(_app *p_app, VkPhysicalDevice physical_device, const VkFormat *candidates, size_t candidate_count, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (size_t i = 0; i < candidate_count; i++) {
		VkFormat format = candidates[i];
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}

	}

	submit_debug_message(
		p_app->inst.instance,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		"formats => failed to find supported formats"
	);
	exit(EXIT_FAILURE);
}

VkFormat find_depth_format(_app *p_app, VkPhysicalDevice physical_device) {
	VkFormat depth_formats[] = {
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	return find_supported_format(
		p_app,
		physical_device,
		depth_formats,
		sizeof(depth_formats) / sizeof(depth_formats[0]),
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool has_stencil_component(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void create_colour_resources(_app *p_app) {
	VkFormat colour_format = p_app->swp.surface_format.format;

	create_image(p_app, &p_app->colour.image, 1, p_app->device.msaa_samples, &p_app->colour.image_allocation, p_app->swp.extent.width, p_app->swp.extent.height, colour_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	create_image_view(p_app, p_app->colour.image, &p_app->colour.image_view, 1, colour_format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void create_depth_resources(_app *p_app) {
	VkFormat depth_format = find_depth_format(p_app, p_app->device.physical);

	create_image(p_app, &p_app->depth.image, 1, p_app->device.msaa_samples, &p_app->depth.image_allocation, p_app->swp.extent.width, p_app->swp.extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	create_image_view(p_app, p_app->depth.image, &p_app->depth.image_view, 1, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
	transition_image_layout(p_app, p_app->depth.image, 1, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void create_mipmaps(_app *p_app, VkImage image, u32 mip_levels, i32 tex_width, i32 tex_height, VkFormat image_format) {

	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(p_app->device.physical, image_format, &format_properties);

	if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"mipmaps => texture image format does not support linear blitting"
		);
		exit(EXIT_FAILURE);
	}

	VkCommandBuffer command_buffer = begin_single_time_commands(p_app);

	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.image = image,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
		.subresourceRange.levelCount = 1,
	};

	i32 mip_width = tex_width;
	i32 mip_height = tex_height;

	for (int i = 1; i < mip_levels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		VkImageBlit blit = {
			.srcOffsets[0] = { 0, 0, 0 },
			.srcOffsets[1] = { mip_width, mip_height, 1 },
			.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.srcSubresource.mipLevel = i - 1,
			.srcSubresource.baseArrayLayer = 0,
			.srcSubresource.layerCount = 1,
			.dstOffsets[0] = { 0, 0, 0 },
			.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 },
			.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.dstSubresource.mipLevel = i,
			.dstSubresource.baseArrayLayer = 0,
			.dstSubresource.layerCount = 1,
		};

		vkCmdBlitImage(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		if (mip_width > 1) mip_width /= 2;
		if (mip_height > 1) mip_height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	end_single_time_commands(p_app, command_buffer);
}

void create_texture_image(_app *p_app) {
	p_app->tex.images = malloc(sizeof(VkImage) * p_app->tex.atlas.count);
	p_app->tex.image_allocations = malloc(sizeof(VmaAllocation) * p_app->tex.atlas.count);
	p_app->tex.image_views = malloc(sizeof(VkImageView) * p_app->tex.atlas.count);
	p_app->tex.mip_levels = malloc(sizeof(u32) * p_app->tex.atlas.count);

	for (u32 i = 0; i < p_app->tex.atlas.count; i++) {
		const char *dir = p_app->config.dir.atlases;
		const char *file = p_app->tex.atlas.names[i];
		size_t len = strlen(dir) + strlen(file) + 1;
		char *path = malloc(len);
		strcpy(path, dir);
		strcat(path, file);

		if (!path || strlen(path) == 0) {
			p_app->tex.images[i] = VK_NULL_HANDLE;
			p_app->tex.image_allocations[i] = NULL;
			p_app->tex.image_views[i] = VK_NULL_HANDLE;
			continue;
		}

		int tex_w, tex_h, tex_channels;
		stbi_uc *pixels = stbi_load(path, &tex_w, &tex_h, &tex_channels, STBI_rgb_alpha);
		if (!pixels) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"texture image => failed to load texture: %s", path
			);
			exit(EXIT_FAILURE);
		}
		free(path);
		p_app->tex.mip_levels[i] = (u32)floor(log2(tex_w > tex_h ? tex_w : tex_h)) + 1;

		VkDeviceSize image_size = tex_w * tex_h * 4;

		VkBuffer staging_buffer;
		VmaAllocation staging_alloc;
		VkBufferCreateInfo buffer_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = image_size,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		VmaAllocationCreateInfo alloc_info = { .usage = VMA_MEMORY_USAGE_CPU_ONLY };
		vmaCreateBuffer(p_app->mem.alloc, &buffer_info, &alloc_info, &staging_buffer, &staging_alloc, NULL);

		void *data;
		vmaMapMemory(p_app->mem.alloc, staging_alloc, &data);
		memcpy(data, pixels, (size_t)image_size);
		vmaUnmapMemory(p_app->mem.alloc, staging_alloc);
		stbi_image_free(pixels);

		create_image(p_app, &p_app->tex.images[i], p_app->tex.mip_levels[i], VK_SAMPLE_COUNT_1_BIT, &p_app->tex.image_allocations[i], tex_w, tex_h, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		transition_image_layout(p_app, p_app->tex.images[i], p_app->tex.mip_levels[i], VK_FORMAT_R8G8B8A8_SRGB,	VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copy_buffer_to_image(p_app, staging_buffer, p_app->tex.images[i], tex_w, tex_h);

		create_mipmaps(p_app, p_app->tex.images[i], p_app->tex.mip_levels[i], tex_w, tex_h, VK_FORMAT_R8G8B8A8_SRGB);

		//transition_image_layout(p_app, p_app->tex.images[i], p_app->tex.mip_levels[i], VK_FORMAT_R8G8B8A8_SRGB,	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vmaDestroyBuffer(p_app->mem.alloc, staging_buffer, staging_alloc);
	}
}

void create_texture_image_view(_app *p_app) {
	for (u32 i = 0; i < p_app->tex.atlas.count; i++) {
		if (p_app->tex.images[i] != VK_NULL_HANDLE) {
			create_image_view(p_app, p_app->tex.images[i], &p_app->tex.image_views[i], p_app->tex.mip_levels[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}
}

void create_texture_sampler(_app *p_app) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(p_app->device.physical, &properties);

	int max = 0;
	for (int i = 0; i < p_app->tex.atlas.count; i++) {
		if (p_app->tex.mip_levels[i] > max) {
			max = p_app->tex.mip_levels[i];
		}
	}

	VkSamplerCreateInfo sampler_create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.mipLodBias = 0.0f,
		.minLod = 0.0f,
		.maxLod = (float)max,
	};

	if (vkCreateSampler(p_app->device.logical, &sampler_create_info, NULL, &p_app->tex.sampler) != VK_SUCCESS) {

		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"texture sampler => failed to create texture sampler"
		);
		exit(EXIT_FAILURE);
	}
}

void create_image(_app *p_app, VkImage *p_image, u32 mip_levels, VkSampleCountFlagBits num_samples, VmaAllocation *p_allocation, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memory_usage) {
	VkImageCreateInfo image_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent.width = width,
		.extent.height = height,
		.extent.depth = 1,
		.mipLevels = mip_levels,
		.arrayLayers = 1,
		.format = format,
		.tiling = tiling,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = usage,
		.samples = num_samples,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	VmaAllocationCreateInfo alloc_create_info = {
		.usage = memory_usage,
	};

	if (vmaCreateImage(p_app->mem.alloc, &image_create_info, &alloc_create_info, p_image, p_allocation, NULL) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"image => failed to create image"
		);
		exit(EXIT_FAILURE);
	}
}

void transition_image_layout(_app *p_app, VkImage image, u32 mip_levels, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {
	VkCommandBuffer command_buffer = begin_single_time_commands(p_app);

	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = mip_levels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.srcAccessMask = 0,
		.dstAccessMask = 0,
	};

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;

	if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (has_stencil_component(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	
	} else {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"layout transition => unsupported layout transition"
		);
		exit(EXIT_FAILURE);
	}

	vkCmdPipelineBarrier(
		command_buffer,
		source_stage, destination_stage,
		0,
		0, NULL,
		0, NULL,
		1, &barrier
	);

	end_single_time_commands(p_app, command_buffer);
}

void copy_buffer_to_image(_app *p_app, VkBuffer buffer, VkImage image, u32 width, u32 height) {

	VkCommandBuffer command_buffer = begin_single_time_commands(p_app);

	VkBufferImageCopy region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {0, 0, 0},
		.imageExtent = {width, height, 1},
	};

	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


	end_single_time_commands(p_app, command_buffer);
}
