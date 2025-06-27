#ifndef DEFINE_H
#define DEFINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>
#include "../libraries/fast_obj.h"
#include "../libraries/cgltf.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../libraries/vk_mem_alloc.h"
#include "../libraries/stb_image.h"
#include "../libraries/stb_image_write.h"
#ifdef __cplusplus
}
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

#define MAX_OBJECT_FILES 16
#define MAX_LIGHTS 16
extern const u32 MAX_FRAMES_IN_FLIGHT;

u32 clamp(u32 n, u32 min, u32 max);

typedef enum {
	ENTRY_TYPE_TRANSPARENT_MESH,
	ENTRY_TYPE_BILLBOARD
} _transparency_entry_type;

typedef struct {
	_transparency_entry_type type;
	u32 index;
	float distance;
} _transparency_entry;

typedef struct _division {
	u16 x;
	u16 y;
	u16 w;
	u16 h;
} _division;

typedef enum _texture_flags {
	TEXTURE_FLAG_NONE       = 0,
	TEXTURE_FLAG_HAS_ALPHA  = 1 << 0,
	TEXTURE_FLAG_IS_ROTATED = 1 << 1,
} _texture_flags;

typedef struct _texture {
	u16 index;
	u16 x;
	u16 y;
	u16 w;
	u16 h;
	u8 flags;
} _texture;

typedef struct _texture_entry {
	u16 w;
	u16 h;
	u16 index;
	stbi_uc *pixels;
} _texture_entry;

typedef struct _textures {
	u16 count;
	_texture_entry *entries;
} _textures;

typedef struct _packer {
	u16 scale;
	_division *divisions;
	u16 division_count;
	_texture *textures;
	u16 texture_count;
} _packer;

typedef struct _vertex {
	float pos[3];
	float tex[2];
	float norm[3];
	int tex_index;
} _vertex;

typedef struct _node {
	_vertex vertex;
	u32 index;
	struct _node* next;
} _node;

typedef struct _candidates {
	VkPhysicalDevice *p_physical_device;
	u32 score;
} _candidates;

typedef struct _billboard {
	vec4 pos;
	vec4 data;
	vec4 flags;
} _billboard;

typedef struct _ubo {
	mat4 proj;
	mat4 view;
	vec4 ambient_light;
	_billboard lights[16];
	int light_count;
} _ubo;

typedef struct _push_constants {
	mat4 model;
	mat4 normal;
} _push_constants;

typedef struct _render_order {
	u32 object_index;
	float distance;
} _render_order;

typedef struct _queue_family_indices {
	u32 graphics_family;
	u32 present_family;
	u32 is_graphics_family_set;
	u32 is_present_family_set;
} _queue_family_indices;

typedef struct _swapchain_support {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR *surface_formats;
	VkPresentModeKHR *present_modes;
	u32 surface_formats_count;
	u32 present_modes_count;
} _swapchain_support;

typedef struct _file_info {
	char **file_names;
	char **atlas_names;
	char *file_dir;
	char *atlas_dir;
	u32 atlas_count;
	u32 file_count;
	u32 *file_to_atlas_map;
} _file_info;

typedef struct _app_window {
	GLFWwindow* window;
} _app_window;

typedef struct _app_instance {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
} _app_instance;

typedef struct _app_device {
	VkPhysicalDevice physical;
	VkDevice logical;
	VkQueue graphics_queue;
	VkQueue present_queue;
	_queue_family_indices queue_indices;
	VkSampleCountFlagBits msaa_samples;
} _app_device;

typedef struct _app_swapchain {
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkImage* images;
	u32 images_count;
	VkImageView* image_views;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D extent;
	bool framebuffer_resized;
} _app_swapchain;

typedef struct _app_pipeline {
	VkRenderPass render_pass;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout layout;
	VkPipeline opaque;
	VkPipeline transparent;
	VkPipeline billboard;
	VkFramebuffer* swapchain_framebuffers;
} _app_pipeline;

typedef struct _app_commands {
	VkCommandPool pool;
	VkCommandBuffer* buffers;
} _app_commands;

typedef struct _app_sync {
	VkSemaphore* image_available_semaphores;
	VkSemaphore* render_finished_semaphores;
	VkFence* in_flight_fences;
	u32 frame_index;
} _app_sync;

typedef struct _app_memory {
	VmaAllocator alloc;
} _app_memory;

typedef struct _app_mesh {
	VkBuffer* vertex_buffers;
	VkBuffer* index_buffers;
	VmaAllocation* vertex_allocations;
	VmaAllocation* index_allocations;
} _app_mesh;

typedef struct _app_billboard {
	VkBuffer instance_buffer;
	VmaAllocation instance_allocation;
	void* buffers_mapped;
} _app_billboard;

typedef struct _app_uniforms {
	VkBuffer* buffers;
	VmaAllocation* buffer_allocations;
	void** buffers_mapped;
} _app_uniforms;

typedef struct _app_descriptors {
	VkDescriptorPool pool;
	VkDescriptorSet* sets;
} _app_descriptors;

typedef struct _app_texture {
	u32 *mip_levels;
	VkImage *images;
	VkImage *atlases;
	VmaAllocation *image_allocations;
	VkImageView *image_views;
	u32 *has_alpha;
	VkSampler sampler;
} _app_texture;

typedef struct _app_depth_resources {
	VkImage image;
	VmaAllocation image_allocation;
	VkImageView image_view;
} _app_depth;

typedef struct _app_colour {
	VkImage image;
	VmaAllocation image_allocation;
	VkImageView image_view;
} _app_colour;

typedef struct _app_config {
	char *win_title;
	u32 win_width;
	u32 win_height;
	char *mesh_vert_shader_path;
	char *mesh_frag_shader_path;
	char *billboard_vert_shader_path;
	char *billboard_frag_shader_path;
	char **object_paths;
	char **gltf_paths;
	u32 gltf_files_count;
	u32 object_files_count;
	_file_info gltf;
} _app_config;

typedef struct _app_objects {
	_vertex** vertices;
	u32** indices;
	u32* vertices_count;
	u32* indices_count;
	u32** face_indices;
	u32** material_indices;
	u32** group_indices;
	u32** object_indices;
	u32** texture_indices;
	fastObjTexture *textures;
	u32 object_count;
	u32 texture_count;
	fastObjMesh *mesh;
	_billboard *lights;
	u32 light_count;
	cgltf_data **data;
	cgltf_options options;
} _app_objects;

typedef struct _app_view {
	vec3 camera_pos;
	vec3 target;
	vec3 world_up;
	float fov_y;
	float near_plane;
	float far_plane;
	float rotation_speed;
	float sensitivity;
	float speed;
	float lerp_speed;
	vec3 cam_offset;
	vec3 cam_offset_goal;
	double last_mouse_x, last_mouse_y;
	bool first_mouse;
	bool mouse_locked;
	float yaw, pitch;
} _app_view;

typedef struct _app_lighting {
	vec4 ambient;
} _app_lighting;

typedef struct _app_performance {
	struct timespec last_frame_time;
	float delta_time;
	float frame_time_avg;
	float fps_avg;
	int frame_count;
} _app_performance;

typedef struct _app {
	_app_window win;
	_app_instance inst;
	_app_device device;
	_app_swapchain swp;
	_app_pipeline pipeline;
	_app_commands cmd;
	_app_sync sync;
	_app_memory mem;
	_app_uniforms uniform;
	_app_descriptors descriptor;
	_app_texture tex;
	_app_depth depth;
	_app_config config;
	_app_mesh mesh;
	_app_billboard billboard;
	_app_objects obj;
	_app_view view;
	_app_colour colour;
	_app_lighting lighting;
	_app_performance perf;
} _app;

#endif
