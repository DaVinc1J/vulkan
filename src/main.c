#include "define.h"

#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif

const char* validation_layers[] = { "VK_LAYER_KHRONOS_validation" };
const u32 validation_layer_count = sizeof(validation_layers) / sizeof(validation_layers[0]);
#ifdef __APPLE__
const char* enabled_logical_device_extensions[] = {
	"VK_KHR_swapchain",
	"VK_KHR_portability_subset"
};
#else
const char* enabled_logical_device_extensions[] = {
	"VK_KHR_swapchain"
};
#endif
const u32 enabled_logical_device_extensions_count = sizeof(enabled_logical_device_extensions) / sizeof(enabled_logical_device_extensions[0]);

const u32 MAX_FRAMES_IN_FLIGHT = 2;
const u32 number_of_mesh_attributes = 4;
const u32 number_of_billboard_attributes = 3;

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data);
VkResult create_debug_utils_messenger_EXT(VkInstance inst, const VkDebugUtilsMessengerCreateInfoEXT *p_create_info, const VkAllocationCallbacks *p_alloc, VkDebugUtilsMessengerEXT *p_debug_messenger);
void destroy_debug_utils_messenger_EXT(VkInstance inst, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *p_alloc);
void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);
void setup_debug_messenger(_app *p_app);
void submit_debug_message(VkInstance inst, VkDebugUtilsMessageSeverityFlagBitsEXT severity, const char* fmt, ...);

u32 clamp(u32 n, u32 min, u32 max) {
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

//// obj //// 
void flatten(_app_obj* p_obj);
void read_obj_file(_app *p_app);

//// GLFW INIT ////
void window_init(_app *p_app);
void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
//// GLFW INIT ////

//// VULKAN INIT ////
// initial vulkan setup //
void vulkan_init(_app *p_app);

// extensions and validation layer check //
bool verify_extensions_support(u32 extensions_count, VkExtensionProperties *extensions, u32 glfw_extensions_count, const char **glfw_extensions);
bool check_validation_layer_support();

// instance and surface //
void create_instance(_app *p_app);
void create_surface(_app *p_app);

// extensions, swapchain, queue families check //
bool is_physical_device_extensions_supported(VkPhysicalDevice physical_device);
void free_swapchain_support(_swapchain_support *p_support);
bool check_swapchain_support(_app *p_app, VkPhysicalDevice physical_device, _swapchain_support *support);
void rate_physical_device_suitability(_candidates *p_candidate);
_queue_family_indices find_queue_families(_app *p_app, VkPhysicalDevice physical_device);
bool is_physical_device_suitable(_app *p_app, VkPhysicalDevice physical_device);

// physical device //
void pick_physical_device(_app *p_app);

// logical device //
void create_logical_device(_app *p_app);

// allocator // 
void create_alloc(_app *p_app);

// choose swapchain settings //
VkSurfaceFormatKHR choose_swapchain_surface_format(_app *p_app, _swapchain_support *p_support);
VkPresentModeKHR choose_swapchain_present_mode(_app *p_app, _swapchain_support *p_support);
VkExtent2D choose_swapchain_swap_extent(_app *p_app, _swapchain_support *p_support);

// swapchain // 
void create_swapchain(_app *p_app);

// image views //
void create_image_view(_app *p_app, VkImage image, VkImageView *p_image_view, u32 mip_levels, VkFormat format, VkImageAspectFlags aspect_flags);
void create_image_views(_app *p_app);

// render pass //
void create_render_pass(_app *p_app);

// descriptor set layout //
void create_descriptor_set_layout(_app *p_app);

// graphics pipeline //
char* read_file(_app *p_app, const char *filename, size_t* shader_code_size);
void create_graphics_pipelines(_app *p_app);
VkShaderModule create_shader_module(_app *p_app, const char* shader_code, size_t shader_code_size);

// command pool //
void create_command_pool(_app *p_app);

// colour resources //
void create_colour_resources(_app *p_app);

// depth resources //
VkFormat find_supported_format(_app *p_app, VkPhysicalDevice physical_device, const VkFormat *candidates, size_t candidate_count, VkImageTiling tiling, VkFormatFeatureFlags features);
VkFormat find_depth_format(_app *p_app, VkPhysicalDevice physical_device);
bool has_stencil_component(VkFormat format);
void create_depth_resources(_app *p_app);

// framebuffers //
void create_framebuffers(_app *p_app);

// texture //
void create_mipmaps(_app *p_app, VkImage image, u32 mip_levels, i32 tex_width, i32 tex_height, VkFormat image_format);
void create_image(_app *p_app, VkImage *p_image, u32 mip_levels, VkSampleCountFlagBits num_samples, VmaAllocation *p_allocation, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memory_usage);
void transition_image_layout(_app *p_app, VkImage image, u32 mip_levels, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
void copy_buffer_to_image(_app *p_app, VkBuffer buffer, VkImage image, u32 width, u32 height);
void create_texture_image(_app *p_app);
void create_texture_image_view(_app *p_app);
void create_texture_sampler(_app *p_app);

// buffers //
void create_buffer(_app *p_app, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VkBuffer *p_buffer, VmaAllocation *p_allocation);
void copy_buffer(_app *p_app, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
void create_billboard_buffer(_app *p_app);
void create_mesh_buffer(_app *p_app);
void create_uniform_buffers(_app *p_app);

// descriptors //
void create_descriptor_pool(_app *p_app);
void create_descriptor_sets(_app *p_app);

// command buffer //
void create_command_buffers(_app *p_app);
void record_command_buffer(_app *p_app, VkCommandBuffer command_buffer, uint32_t image_index);
VkCommandBuffer begin_single_time_commands(_app *p_app);
void end_single_time_commands(_app *p_app, VkCommandBuffer command_buffer);

// sync objects //
void create_sync_objects(_app *p_app);

//// VULKAN INIT ////

//// MAIN LOOP ////
void main_loop(_app *p_app);
void draw_frame(_app *p_app);
void cleanup_swapchain(_app *p_app);
void recreate_swapchain(_app *p_app);
void update_uniform_buffer(_app *p_app, u32 current_image);
float get_delta_time();
void update_view(_app *p_app, float time);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
//// MAIN LOOP ////

//// CLEAN UP ////
void clean(_app *p_app);
//// CLEAN UP ////

//// VERTEX READ ////

VkVertexInputBindingDescription get_mesh_binding_description() {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(_vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_description;
}

VkVertexInputBindingDescription get_billboard_binding_description() {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(_billboard);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	return binding_description;
}

void get_mesh_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs) {
	if (attribs == NULL) {
		*num_attribs = number_of_mesh_attributes;
		return;
	}

	attribs[0].binding = 0;
	attribs[0].location = 0;
	attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribs[0].offset = offsetof(_vertex, pos);

	attribs[1].binding = 0;
	attribs[1].location = 1;
	attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
	attribs[1].offset = offsetof(_vertex, tex);

	attribs[2].binding = 0;
	attribs[2].location = 2;
	attribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribs[2].offset = offsetof(_vertex, norm);

	attribs[3].binding = 0;
	attribs[3].location = 3;
	attribs[3].format = VK_FORMAT_R32_SINT;
	attribs[3].offset = offsetof(_vertex, tex_index);
}

void get_billboard_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs) {
	if (attribs == NULL) {
		*num_attribs = number_of_billboard_attributes;
		return;
	}

	attribs[0].binding = 0;
	attribs[0].location = 0;
	attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribs[0].offset = offsetof(_billboard, position);

	attribs[1].binding = 0;
	attribs[1].location = 1;
	attribs[1].format = VK_FORMAT_R32_SFLOAT;
	attribs[1].offset = offsetof(_billboard, size);

	attribs[2].binding = 0;
	attribs[2].location = 2;
	attribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribs[2].offset = offsetof(_billboard, color);
}

//// VERTEX READ ////

void read_obj_file(_app *p_app) {
	u32 file_count = p_app->config.object_files_count;
	_app_obj *fillers = calloc(file_count, sizeof(_app_obj));

	p_app->obj.object_count = 0;
	p_app->obj.texture_count = 0;

	// First pass: load and flatten
	for (u32 i = 0; i < file_count; i++) {
		fillers[i].mesh = fast_obj_read(p_app->config.object_paths[i]);
		u32 oc = fillers[i].mesh->object_count;

		fillers[i].vertices = malloc(oc * sizeof(_vertex*));
		fillers[i].indices = malloc(oc * sizeof(u32*));
		fillers[i].face_indices = malloc(oc * sizeof(u32*));
		fillers[i].material_indices = malloc(oc * sizeof(u32*));
		fillers[i].group_indices = malloc(oc * sizeof(u32*));
		fillers[i].object_indices = malloc(oc * sizeof(u32*));
		fillers[i].texture_indices = malloc(oc * sizeof(u32*));

		fillers[i].vertices_count = malloc(oc * sizeof(u32));
		fillers[i].indices_count = malloc(oc * sizeof(u32));

		flatten(&fillers[i]);
		p_app->obj.object_count += oc;
		p_app->obj.texture_count += fillers[i].mesh->texture_count - 1;
	}

	_app_obj *dst = &p_app->obj;
	dst->mesh = NULL;
	dst->vertices = malloc(dst->object_count * sizeof(_vertex*));
	dst->indices = malloc(dst->object_count * sizeof(u32*));
	dst->face_indices = malloc(dst->object_count * sizeof(u32*));
	dst->material_indices = malloc(dst->object_count * sizeof(u32*));
	dst->group_indices = malloc(dst->object_count * sizeof(u32*));
	dst->object_indices = malloc(dst->object_count * sizeof(u32*));
	dst->texture_indices = malloc(dst->object_count * sizeof(u32*));
	dst->vertices_count = malloc(dst->object_count * sizeof(u32));
	dst->indices_count = malloc(dst->object_count * sizeof(u32));
	dst->textures = malloc(dst->texture_count * sizeof(fastObjTexture));

	u32 dst_i = 0;
	u32 tex_offset = 0;
	for (u32 i = 0; i < file_count; i++) {
		u32 oc = fillers[i].mesh->object_count;
		u32 tc = fillers[i].mesh->texture_count;

		for (u32 t = 1; t < tc; t++) {
			dst->textures[tex_offset + t - 1] = fillers[i].mesh->textures[t];
			if (fillers[i].mesh->textures[t].path)
				dst->textures[tex_offset + t - 1].path = strdup(fillers[i].mesh->textures[t].path);
			if (fillers[i].mesh->textures[t].name)
				dst->textures[tex_offset + t - 1].name = strdup(fillers[i].mesh->textures[t].name);
		}

		for (u32 j = 0; j < oc; j++, dst_i++) {
			u32 v_count = fillers[i].vertices_count[j];
			u32 i_count = fillers[i].indices_count[j];

			dst->vertices[dst_i] = malloc(sizeof(_vertex) * v_count);
			dst->indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->face_indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->material_indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->group_indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->object_indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->texture_indices[dst_i] = malloc(sizeof(u32) * i_count);

			memcpy(dst->vertices[dst_i], fillers[i].vertices[j], sizeof(_vertex) * v_count);
			memcpy(dst->indices[dst_i], fillers[i].indices[j], sizeof(u32) * i_count);
			memcpy(dst->face_indices[dst_i], fillers[i].face_indices[j], sizeof(u32) * i_count);
			memcpy(dst->material_indices[dst_i], fillers[i].material_indices[j], sizeof(u32) * i_count);
			memcpy(dst->group_indices[dst_i], fillers[i].group_indices[j], sizeof(u32) * i_count);
			memcpy(dst->object_indices[dst_i], fillers[i].object_indices[j], sizeof(u32) * i_count);

			for (u32 k = 0; k < i_count; k++) {
				u32 tex_id = fillers[i].texture_indices[j][k];
				dst->texture_indices[dst_i][k] = tex_id > 0 ? (tex_offset + tex_id - 1) : UINT32_MAX;
			}

			dst->vertices_count[dst_i] = v_count;
			dst->indices_count[dst_i] = i_count;
		}

		fast_obj_destroy(fillers[i].mesh);
		free(fillers[i].vertices);
		free(fillers[i].indices);
		free(fillers[i].face_indices);
		free(fillers[i].material_indices);
		free(fillers[i].group_indices);
		free(fillers[i].object_indices);
		free(fillers[i].texture_indices);
		free(fillers[i].vertices_count);
		free(fillers[i].indices_count);

		tex_offset += tc - 1;
	}
	free(fillers);

	for (u32 o = 0; o < dst->object_count; o++) {
		for (u32 k = 0; k < dst->indices_count[o]; k++) {
			u32 idx = dst->indices[o][k];
			dst->vertices[o][idx].tex_index = dst->texture_indices[o][k];
		}
	}

	u32 tex_count = dst->texture_count;
	u32 *remap_table = malloc(sizeof(u32) * tex_count);
	u32 new_index = 0;

	for (u32 i = 0; i < tex_count; i++) {
		const char *path = dst->textures[i].path;
		if (path && strlen(path) > 0) {
			remap_table[i] = new_index++;
		} else {
			remap_table[i] = UINT32_MAX;
		}
	}

	for (u32 o = 0; o < dst->object_count; o++) {
		for (u32 v = 0; v < dst->vertices_count[o]; v++) {
			u32 old = dst->vertices[o][v].tex_index;
			dst->vertices[o][v].tex_index = (old < tex_count && remap_table[old] != UINT32_MAX) ? remap_table[old] : 0;
		}
	}
	free(remap_table);
}

///////////////////////////////////////////
//////////////// MAIN LOOP ////////////////
///////////////////////////////////////////
int main() {
	_app app = {0};
	app.config.win_title = "davincij";
	app.config.win_width = 800;
	app.config.win_height = 800;
	app.config.mesh_vert_shader_path = "src/shaders/mesh.vert.spv";
	app.config.mesh_frag_shader_path = "src/shaders/mesh.frag.spv";
	app.config.billboard_vert_shader_path = "src/shaders/billboard.vert.spv";
	app.config.billboard_frag_shader_path = "src/shaders/billboard.frag.spv";

	char* object_paths[] = {
		"src/objects/viking_room.obj",
	};

	app.config.object_files_count = (u32)(sizeof(object_paths) / sizeof(object_paths[0]));
	app.config.object_paths = malloc(sizeof(char*) * app.config.object_files_count);
	for (int i = 0; i < app.config.object_files_count; i++) {
		app.config.object_paths[i] = object_paths[i];
	}

	glm_vec3_copy((vec3){2.0f, 2.0f, 2.0f}, app.view.camera_pos);
	glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, app.view.target);
	glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, app.view.world_up);
	app.view.fov_y = 80.0f;
	app.view.near_plane = 0.01f;
	app.view.far_plane = 1000.0f;
	app.view.rotation_speed = 0.0f;
	app.view.speed = 0.1f;
	app.view.lerp_speed = 1.0f;
	app.view.sensitivity = 0.2f;
	app.view.yaw = -90.0f;
	app.view.pitch = 0.0f;
	app.view.first_mouse = true;
	app.view.mouse_locked = true;

	_billboard billboards[] = {
	{ .position = {1, 2, 1}, .size = 3.0f, .color = {1, 1, 0, 1} },
};
	
	app.obj.billboard_count = sizeof(billboards) / sizeof(billboards[0]);
	app.obj.billboards = billboards;

	window_init(&app);
	vulkan_init(&app);
	main_loop(&app);
	clean(&app);
}

///////////////////////////////////////////
//////////////// GLFW INIT ////////////////
///////////////////////////////////////////

//////// glfw main loop ////////
void window_init(_app *p_app) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	p_app->win.window = glfwCreateWindow(p_app->config.win_width, p_app->config.win_height, p_app->config.win_title, NULL, NULL);

	glfwSetWindowUserPointer(p_app->win.window, p_app);
	glfwSetFramebufferSizeCallback(p_app->win.window, framebuffer_resize_callback);
	glfwSetInputMode(p_app->win.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(p_app->win.window, mouse_callback);
	glfwSetMouseButtonCallback(p_app->win.window, mouse_button_callback);
}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);
	p_app->swp.framebuffer_resized = true;
}

/////////////////////////////////////////////
//////////////// VULKAN INIT ////////////////
/////////////////////////////////////////////

//////// vulkan main loop ////////
//////////////////////////////////
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

//////// extensions and validation layer check ////////
///////////////////////////////////////////////////////

//// extensions check ////
bool verify_extensions_support(u32 extensions_count, VkExtensionProperties *extensions, u32 glfw_extensions_count, const char **glfw_extensions) {

	for (int i = 0; i < glfw_extensions_count; i++) {
		bool extension_found = false;
		for (int j = 0; j < extensions_count; j++) {
			if (strcmp(extensions[j].extensionName, glfw_extensions[i]) == 0) {
				extension_found = true;
				break;
			}
		}
		if (!extension_found) {
			return false;
		}
	}

	return true;
}

//// validation layer check ////
bool check_validation_layer_support() {
	u32 layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	VkLayerProperties available_layers[layer_count];
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

	bool layer_found = false;
	for (u32 i = 0; i < validation_layer_count; i++) {
		layer_found = false;
		for (u32 j = 0;j < layer_count; j++) {
			if (strcmp(available_layers[j].layerName, validation_layers[i]) == 0) {
				layer_found = true;
				break;
			}
		}
		if (!layer_found) {
			return false;
		}
	}
	return true;
}

//////// create instance and surface ////////
/////////////////////////////////////////////

//// create instance ////
void create_instance(_app *p_app) {
	if (enable_validation_layers && !check_validation_layer_support()) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"validation layers requested but no available"
		);
		exit(EXIT_FAILURE);
	}

	u32 glfw_extensions_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

	u32 required_extensions_count = glfw_extensions_count + (enable_validation_layers ? 1 : 0);
#ifdef __APPLE__
	required_extensions_count += 1;
#endif

	const char *required_extensions[required_extensions_count];

	for (u32 i = 0; i < glfw_extensions_count; i++) {
		required_extensions[i] = glfw_extensions[i];
	}

	u32 ext_i = glfw_extensions_count;

#ifdef __APPLE__
	required_extensions[ext_i++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
#endif

	if (enable_validation_layers) {
		required_extensions[ext_i++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

	u32 api_version = 0;
	vkEnumerateInstanceVersion(&api_version);

	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = p_app->config.win_title,
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = p_app->config.win_title,
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = api_version,
		.pNext = NULL,
	};

	VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledExtensionCount = required_extensions_count,
		.ppEnabledExtensionNames = required_extensions,
	};

#ifdef __APPLE__
	create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
	if (enable_validation_layers) {
		create_info.enabledLayerCount = validation_layer_count;
		create_info.ppEnabledLayerNames = validation_layers;
		populate_debug_messenger_create_info(&debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext = NULL;
	}

	if (vkCreateInstance(&create_info, NULL, &p_app->inst.instance) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"instance => failed to create instance"
		);
		exit(EXIT_FAILURE);
	}

	u32 extensions_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extensions_count, NULL);
	VkExtensionProperties extensions[extensions_count];
	vkEnumerateInstanceExtensionProperties(NULL, &extensions_count, extensions);

	if (!verify_extensions_support(extensions_count, extensions, glfw_extensions_count, glfw_extensions)) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"instance => extensions support verify failed"
		);
		exit(EXIT_FAILURE);
	}
}

//// create surface ////
void create_surface(_app *p_app) {
	if (glfwCreateWindowSurface(p_app->inst.instance, p_app->win.window, NULL, &p_app->swp.surface) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"surface => failed to create surface"
		);
		exit(EXIT_FAILURE);
	}
}

//////// extensions, swapchain and queue families check ////////
////////////////////////////////////////////////////////////////

//// extensions check ////
bool is_physical_device_extensions_supported(VkPhysicalDevice physical_device) {

	u32 extensions_count;
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_count, NULL);

	VkExtensionProperties available_extensions[extensions_count];
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_count, available_extensions);

	for (u32 i = 0; i < enabled_logical_device_extensions_count; i++) {
		bool extension_found = false;
		for (u32 j = 0; j < extensions_count; j++) {
			if (strcmp(enabled_logical_device_extensions[i], available_extensions[j].extensionName) == 0) {
				extension_found = true;
				break;
			}
		}
		if (!extension_found) {
			return false;
		}
	}

	return true;
}

void free_swapchain_support(_swapchain_support *p_support) {
	free(p_support->surface_formats);
	p_support->surface_formats = NULL;
	free(p_support->present_modes);
	p_support->present_modes = NULL;
}

//// swapchain check ////
bool check_swapchain_support(_app *p_app, VkPhysicalDevice physical_device, _swapchain_support *p_support) {

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, p_app->swp.surface, &p_support->capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, p_app->swp.surface, &p_support->surface_formats_count, NULL);
	if (p_support->surface_formats_count != 0) {
		p_support->surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * p_support->surface_formats_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, p_app->swp.surface, &p_support->surface_formats_count, p_support->surface_formats);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, p_app->swp.surface, &p_support->present_modes_count, NULL);
	if (p_support->present_modes_count != 0) {
		p_support->present_modes = malloc(sizeof(VkPresentModeKHR) * p_support->present_modes_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, p_app->swp.surface, &p_support->present_modes_count, p_support->present_modes);
	}

	return (p_support->surface_formats_count != 0 && p_support->present_modes_count != 0);
}

//// rate physical devices ////
void rate_physical_device_suitability(_candidates *p_candidate) {
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceProperties(*p_candidate->p_physical_device, &physical_device_properties);
	vkGetPhysicalDeviceFeatures(*p_candidate->p_physical_device, &physical_device_features);

	p_candidate->score = 0;

	if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		p_candidate->score += 1000;
	}

	p_candidate->score += physical_device_properties.limits.maxImageDimension2D;

	if (physical_device_features.geometryShader) {
		p_candidate->score += 200;
	}

	// in the future to improve performance we should look for queue 
	// families that have both graphics and present support
	// as to not have them split, but atp it dont matter
}

//// queue families check ////
_queue_family_indices find_queue_families(_app *p_app, VkPhysicalDevice physical_device) {
	_queue_family_indices indices;
	indices.is_graphics_family_set = 0;

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

	VkQueueFamilyProperties queue_family_properties[queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties);

	VkBool32 present_support = false;
	for (int i = 0; i < queue_family_count; i++) {
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
			indices.is_graphics_family_set = 1;
		}
		present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, p_app->swp.surface, &present_support);
		if (present_support) {
			indices.present_family = i;
			indices.is_present_family_set = 1;
		}
	}

	return indices;
}

//// physical device suitability check ////
bool is_physical_device_suitable(_app *p_app, VkPhysicalDevice physical_device) {
	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

	if (!physical_device_features.geometryShader) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
			"physical device => lacks geometry shader support"
		);
	}

	if (!physical_device_features.samplerAnisotropy) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => lacks sampler anisotropy"
		);
		return false;
	}

	_queue_family_indices indices = find_queue_families(p_app, physical_device);
	if (indices.is_graphics_family_set == 0 || indices.is_present_family_set == 0) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find queue families"
		);
		return false;
	}

	if (!is_physical_device_extensions_supported(physical_device)) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => extensions not supported"
		);
		return false;
	}

	_swapchain_support support;
	if (!check_swapchain_support(p_app, physical_device, &support)) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => swapchain not supported"
		);
		return false;
	}
	free_swapchain_support(&support);

	return true;
}

//////// pick physical device ////////
//////////////////////////////////////
//// get sample count ////
void get_max_usable_sample_count(VkPhysicalDevice physical_device, VkSampleCountFlagBits *p_msaa_samples) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts &
		properties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) *p_msaa_samples = VK_SAMPLE_COUNT_64_BIT;
	else if (counts & VK_SAMPLE_COUNT_32_BIT) *p_msaa_samples = VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT) *p_msaa_samples = VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT)  *p_msaa_samples = VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT)  *p_msaa_samples = VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT)  *p_msaa_samples = VK_SAMPLE_COUNT_2_BIT;
	else *p_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
}

//// pick device ////
void pick_physical_device(_app *p_app) {
	p_app->device.physical = VK_NULL_HANDLE;

	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(p_app->inst.instance, &physical_device_count, NULL);

	if (physical_device_count == 0) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find any devices"
		);
		exit(EXIT_FAILURE);
	}

	VkPhysicalDevice physical_devices[physical_device_count];
	vkEnumeratePhysicalDevices(p_app->inst.instance, &physical_device_count, physical_devices);

	if (physical_device_count == 1) {
		if (!is_physical_device_suitable(p_app, physical_devices[0])) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"physical device => only device is not suitable"
			);
			exit(EXIT_FAILURE);
		}

		p_app->device.physical = physical_devices[0];
		get_max_usable_sample_count(p_app->device.physical, &p_app->device.msaa_samples);
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
			"physical device => only device is suitable"
		);
		return;
	}

	_candidates candidates[physical_device_count];

	int max_score = 0;
	int max_index = -1;

	for (int i = 0; i < physical_device_count; i++) {
		if (is_physical_device_suitable(p_app, physical_devices[i])) {
			candidates[i].p_physical_device = &physical_devices[i];
			rate_physical_device_suitability(&candidates[i]);

			if (candidates[i].score > max_score) {
				max_score = candidates[i].score;
				max_index = i;
			}
		}
	}

	if (max_index != -1) {
		p_app->device.physical = *candidates[max_index].p_physical_device;
		get_max_usable_sample_count(p_app->device.physical, &p_app->device.msaa_samples);
	}

	if (p_app->device.physical == VK_NULL_HANDLE) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find a suitable device"
		);
		exit(EXIT_FAILURE);
	}
}

//////// create logical device ////////
///////////////////////////////////////
void create_logical_device(_app *p_app) {
	p_app->device.queue_indices = find_queue_families(p_app, p_app->device.physical);

	u32 unique_queue_families[2] = {
		p_app->device.queue_indices.graphics_family,
		p_app->device.queue_indices.present_family,
	};

	bool same_family = (unique_queue_families[0] == unique_queue_families[1]);

	VkDeviceQueueCreateInfo queue_create_infos[2];
	u32 queue_create_info_count = same_family ? 1 : 2;

	float queue_priority = 1.0f;

	queue_create_infos[0] = (VkDeviceQueueCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = unique_queue_families[0],
		.queueCount = 1,
		.pQueuePriorities = &queue_priority,
	};

	if (!same_family) {
		queue_create_infos[1] = (VkDeviceQueueCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = unique_queue_families[1],
			.queueCount = 1,
			.pQueuePriorities = &queue_priority
		};
	}

	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceFeatures(p_app->device.physical, &physical_device_features);
	physical_device_features.samplerAnisotropy = VK_TRUE;
	physical_device_features.robustBufferAccess = VK_FALSE;

	VkPhysicalDeviceVulkan12Features features_1_2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.runtimeDescriptorArray = VK_TRUE,
		.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
	};

	VkDeviceCreateInfo logical_device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = queue_create_info_count,
		.pQueueCreateInfos = queue_create_infos,
		.pEnabledFeatures = &physical_device_features,
		.enabledExtensionCount = enabled_logical_device_extensions_count,
		.ppEnabledExtensionNames = enabled_logical_device_extensions,
		.pNext = &features_1_2,
	};

	logical_device_create_info.enabledLayerCount = 0;

	if (enable_validation_layers) {
		logical_device_create_info.enabledLayerCount = validation_layer_count;
		logical_device_create_info.ppEnabledLayerNames = validation_layers;
	}

	if (vkCreateDevice(p_app->device.physical, &logical_device_create_info, NULL, &p_app->device.logical) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"logical device => failed to create logical device"
		);
		exit(EXIT_FAILURE);
	}

	vkGetDeviceQueue(p_app->device.logical, p_app->device.queue_indices.graphics_family, 0, &p_app->device.graphics_queue);
	vkGetDeviceQueue(p_app->device.logical, p_app->device.queue_indices.present_family, 0, &p_app->device.present_queue);
}

//////// create alloc ////////
//////////////////////////////
void create_alloc(_app *p_app) {

	VmaVulkanFunctions vulkan_funcs = {
		.vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
		.vkGetDeviceProcAddr = &vkGetDeviceProcAddr,
	};

	u32 api_version = 0;
	vkEnumerateInstanceVersion(&api_version);

	VmaAllocatorCreateInfo allocatorCreateInfo = {
		.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
		.vulkanApiVersion = api_version,
		.physicalDevice = p_app->device.physical,
		.device = p_app->device.logical,
		.instance = p_app->inst.instance,
		.pVulkanFunctions = &vulkan_funcs,
	};

	if (vmaCreateAllocator(&allocatorCreateInfo, &p_app->mem.alloc) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance, 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"allocator => failed to create allocator"
		);
		exit(EXIT_FAILURE);
	}
}


//////// choose swapchain settings ////////
//////////////////////////////////////////

//// choose swapchain surface format ////
VkSurfaceFormatKHR choose_swapchain_surface_format(_app *p_app, _swapchain_support *p_support) {

	for (u32 i = 0; i < p_support->surface_formats_count; i++) {
		if (p_support->surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && p_support->surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return p_support->surface_formats[i];
		}
	}

	submit_debug_message(
		p_app->inst.instance,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		"swapchain surface format => desired surface format not found, falling back to first specified surface format"
	);
	return p_support->surface_formats[0];
}

//// choose swapchain present mode ////
VkPresentModeKHR choose_swapchain_present_mode(_app *p_app, _swapchain_support *p_support) {
	for (u32 i = 0; i  < p_support->present_modes_count; i++) {
		if (p_support->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return p_support->present_modes[i];
		}
	}

	submit_debug_message(
		p_app->inst.instance,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		"swapchain present mode => mailbox present mode not found, falling back to fifo"
	);
	return VK_PRESENT_MODE_FIFO_KHR;
}

//// choose swapchain swap extent //// 
VkExtent2D choose_swapchain_swap_extent(_app *p_app, _swapchain_support *p_support) {
	if (p_support->capabilities.currentExtent.width != UINT32_MAX) {
		return p_support->capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(p_app->win.window, &width, &height);

		VkExtent2D actual_extent = { (u32)width, (u32)height };

		actual_extent.width = clamp(actual_extent.width, p_support->capabilities.minImageExtent.width, p_support->capabilities.maxImageExtent.width);
		actual_extent.height = clamp(actual_extent.height, p_support->capabilities.minImageExtent.height, p_support->capabilities.maxImageExtent.height);

		return actual_extent;
	}
}

//////// create swapchain ////////
//////////////////////////////////
void create_swapchain(_app *p_app) {
	_swapchain_support support;
	check_swapchain_support(p_app, p_app->device.physical, &support);

	p_app->swp.surface_format = choose_swapchain_surface_format(p_app, &support);
	VkPresentModeKHR present_mode = choose_swapchain_present_mode(p_app, &support);
	p_app->swp.extent = choose_swapchain_swap_extent(p_app, &support);

	u32 image_count = support.capabilities.minImageCount + 1;
	if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
		image_count = support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = p_app->swp.surface,
		.minImageCount = image_count,
		.imageFormat = p_app->swp.surface_format.format,
		.imageColorSpace = p_app->swp.surface_format.colorSpace,
		.imageExtent = p_app->swp.extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = support.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	u32 queue_family_indices[] = {p_app->device.queue_indices.graphics_family, p_app->device.queue_indices.present_family};

	if (p_app->device.queue_indices.graphics_family != p_app->device.queue_indices.present_family) {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = NULL;
	}

	if (vkCreateSwapchainKHR(p_app->device.logical, &swapchain_create_info, NULL, &p_app->swp.swapchain) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to create swapchain"
		);
	}

	p_app->swp.images_count = 0;
	vkGetSwapchainImagesKHR(p_app->device.logical, p_app->swp.swapchain, &p_app->swp.images_count, NULL);
	p_app->swp.images = malloc(sizeof(VkImage) * p_app->swp.images_count);
	vkGetSwapchainImagesKHR(p_app->device.logical, p_app->swp.swapchain, &p_app->swp.images_count, p_app->swp.images);
}

//////// create image views ////////
////////////////////////////////////

void create_image_view(_app *p_app, VkImage image, VkImageView *p_image_view, u32 mip_levels, VkFormat format, VkImageAspectFlags aspect_flags) {
	VkImageViewCreateInfo view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = aspect_flags,
			.baseMipLevel = 0,
			.levelCount = mip_levels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	if (vkCreateImageView(p_app->device.logical, &view_create_info, NULL, p_image_view) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"image view => failed to create image view"
		);
		exit(EXIT_FAILURE);
	}
}

void create_image_views(_app *p_app) {
	p_app->swp.image_views = malloc(sizeof(VkImageView) * p_app->swp.images_count);

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		create_image_view(
			p_app,
			p_app->swp.images[i],
			&p_app->swp.image_views[i],
			1,
			p_app->swp.surface_format.format,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}
}

//////// create render pass ////////
////////////////////////////////////
void create_render_pass(_app *p_app) {
	VkAttachmentDescription colour_attachment_description = {
		.format = p_app->swp.surface_format.format,
		.samples = p_app->device.msaa_samples,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference colour_attachment_reference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentDescription colour_attachment_resolve = {
		.format = p_app->swp.surface_format.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference colour_attachment_resolve_reference = {
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentDescription depth_attachment_description = {
		.format = find_depth_format(p_app, p_app->device.physical),
		.samples = p_app->device.msaa_samples,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference depth_attachment_reference = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colour_attachment_reference,
		.pResolveAttachments = &colour_attachment_resolve_reference,
		.pDepthStencilAttachment = &depth_attachment_reference,
	};


	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = 0,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};

	VkAttachmentDescription attachments[] = {colour_attachment_description, depth_attachment_description, colour_attachment_resolve};

	VkRenderPassCreateInfo render_pass_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = sizeof(attachments) / sizeof(attachments[0]),
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	if (vkCreateRenderPass(p_app->device.logical, &render_pass_create_info, NULL, &p_app->pipeline.render_pass) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"render pass => failed to create render pass"
		);
		exit(EXIT_FAILURE);
	}
}


//////// create descriptor set layout ////////
//////////////////////////////////////////////
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

//////// create graphics pipeline ////////
//////////////////////////////////////////
//// read file ////
char* read_file(_app *p_app, const char* filename, size_t* shader_code_size) {
	FILE* p_file = fopen(filename, "rb");
	if (!p_file) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"shader read => failed to open file"
		);
		return NULL;
	}

	fseek(p_file, 0, SEEK_END);
	long file_size = ftell(p_file);
	rewind(p_file);

	if (file_size <= 0) {
		fclose(p_file);
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"shader read => empty or invalid file"
		);
		return NULL;
	}

	char* buffer = malloc(file_size);

	fread(buffer, 1, file_size, p_file);
	fclose(p_file);

	*shader_code_size = (size_t)file_size;
	return buffer;
}

//// create pipeline ////
void create_graphics_pipelines(_app *p_app) {
	size_t mesh_vert_shader_code_size;
	size_t mesh_frag_shader_code_size;
	size_t billboard_vert_shader_code_size;
	size_t billboard_frag_shader_code_size;

	const char* mesh_vert_shader_code = read_file(p_app, p_app->config.mesh_vert_shader_path, &mesh_vert_shader_code_size);
	const char* mesh_frag_shader_code = read_file(p_app, p_app->config.mesh_frag_shader_path, &mesh_frag_shader_code_size);
	const char* billboard_vert_shader_code = read_file(p_app, p_app->config.billboard_vert_shader_path, &billboard_vert_shader_code_size);
	const char* billboard_frag_shader_code = read_file(p_app, p_app->config.billboard_frag_shader_path, &billboard_frag_shader_code_size);

	VkShaderModule mesh_vert_shader_module = create_shader_module(p_app, mesh_vert_shader_code, mesh_vert_shader_code_size); 
	VkShaderModule mesh_frag_shader_module = create_shader_module(p_app, mesh_frag_shader_code, mesh_frag_shader_code_size);
	VkShaderModule billboard_vert_shader_module = create_shader_module(p_app, billboard_vert_shader_code, billboard_vert_shader_code_size); 
	VkShaderModule billboard_frag_shader_module = create_shader_module(p_app, billboard_frag_shader_code, billboard_frag_shader_code_size);

	VkPipelineShaderStageCreateInfo mesh_shader_stages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = mesh_vert_shader_module, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = mesh_frag_shader_module, .pName = "main" },
	};

	VkPipelineShaderStageCreateInfo billboard_shader_stages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = billboard_vert_shader_module, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = billboard_frag_shader_module, .pName = "main" },
	};

	VkVertexInputBindingDescription mesh_binding_desc = get_mesh_binding_description();
	u32 mesh_attr_count = 0;
	get_mesh_attribute_descriptions(NULL, &mesh_attr_count);
	VkVertexInputAttributeDescription mesh_attr_descs[mesh_attr_count];
	get_mesh_attribute_descriptions(mesh_attr_descs, NULL);

	VkVertexInputBindingDescription billboard_binding_desc = get_billboard_binding_description();
	u32 billboard_attr_count = 0;
	get_billboard_attribute_descriptions(NULL, &billboard_attr_count);
	VkVertexInputAttributeDescription billboard_attr_descs[billboard_attr_count];
	get_billboard_attribute_descriptions(billboard_attr_descs, NULL);

	VkPipelineVertexInputStateCreateInfo mesh_vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.vertexAttributeDescriptionCount = mesh_attr_count,
		.pVertexBindingDescriptions = &mesh_binding_desc,
		.pVertexAttributeDescriptions = mesh_attr_descs,
	};

	VkPipelineVertexInputStateCreateInfo billboard_vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.vertexAttributeDescriptionCount = billboard_attr_count,
		.pVertexBindingDescriptions = &billboard_binding_desc,
		.pVertexAttributeDescriptions = billboard_attr_descs,
	};

	VkPipelineInputAssemblyStateCreateInfo input_asm = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	VkViewport viewport = {
		.width = p_app->swp.extent.width,
		.height = p_app->swp.extent.height,
		.minDepth = 0.0f, .maxDepth = 1.0f,
	};

	VkRect2D scissor = { .extent = p_app->swp.extent };

	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1, .pViewports = &viewport,
		.scissorCount = 1, .pScissors = &scissor,
	};

	VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamic_states,
	};

	VkPipelineRasterizationStateCreateInfo raster = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisample = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = p_app->device.msaa_samples,
	};

	VkPipelineDepthStencilStateCreateInfo depth = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
	};

	VkPipelineColorBlendAttachmentState blend_opaque = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE
	};

	VkPipelineColorBlendAttachmentState blend_transparent = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendAttachmentState blend_billboard = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendStateCreateInfo blend_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
	};

	VkPushConstantRange push_constant_range = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.offset = 0,
		.size = sizeof(_push_constants),
	};

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &p_app->pipeline.descriptor_set_layout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range,
	};

	if (vkCreatePipelineLayout(p_app->device.logical, &layout_info, NULL, &p_app->pipeline.layout) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "pipeline layout => failed");
		exit(EXIT_FAILURE);
	}

	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = mesh_shader_stages,
		.pVertexInputState = &mesh_vertex_input,
		.pInputAssemblyState = &input_asm,
		.pViewportState = &viewport_state,
		.pRasterizationState = &raster,
		.pMultisampleState = &multisample,
		.pDepthStencilState = &depth,
		.pDynamicState = &dynamic_state,
		.layout = p_app->pipeline.layout,
		.renderPass = p_app->pipeline.render_pass,
	};

	depth.depthWriteEnable = VK_TRUE;
	blend_state.pAttachments = &blend_opaque;
	pipeline_info.pColorBlendState = &blend_state;
	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &p_app->pipeline.opaque) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "opaque pipeline => failed");
		exit(EXIT_FAILURE);
	}

	depth.depthWriteEnable = VK_FALSE;
	blend_state.pAttachments = &blend_transparent;
	pipeline_info.pColorBlendState = &blend_state;
	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &p_app->pipeline.transparent) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "transparent pipeline => failed");
		exit(EXIT_FAILURE);
	}

	depth.depthWriteEnable = VK_FALSE;
	blend_state.pAttachments = &blend_billboard;
	pipeline_info.pColorBlendState = &blend_state;
	pipeline_info.pStages = billboard_shader_stages;
	pipeline_info.pVertexInputState = &billboard_vertex_input;
	if (vkCreateGraphicsPipelines(p_app->device.logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &p_app->pipeline.billboard) != VK_SUCCESS) {
		submit_debug_message(p_app->inst.instance, VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "billboard pipeline => failed");
		exit(EXIT_FAILURE);
	}

	vkDestroyShaderModule(p_app->device.logical, mesh_frag_shader_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, mesh_vert_shader_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, billboard_frag_shader_module, NULL);
	vkDestroyShaderModule(p_app->device.logical, billboard_vert_shader_module, NULL);
}

//// create shader module ////
VkShaderModule create_shader_module(_app *p_app, const char* shader_code, size_t shader_code_size) {
	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shader_code_size,
		.pCode = (const uint32_t*) shader_code,
	};

	VkShaderModule shader_module;
	if (vkCreateShaderModule(p_app->device.logical, &shader_module_create_info, NULL, &shader_module) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst.instance,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"shader module => failed to create shader module"
		);
		exit(EXIT_FAILURE);
	}

	return shader_module;
}


//////// command pool ////////
//////////////////////////////
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

//////// depth resources ////////
/////////////////////////////////

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

//////// colour resources ////////
//////////////////////////////////
void create_colour_resources(_app *p_app) {
	VkFormat colour_format = p_app->swp.surface_format.format;

	create_image(p_app, &p_app->colour.image, 1, p_app->device.msaa_samples, &p_app->colour.image_allocation, p_app->swp.extent.width, p_app->swp.extent.height, colour_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	create_image_view(p_app, p_app->colour.image, &p_app->colour.image_view, 1, colour_format, VK_IMAGE_ASPECT_COLOR_BIT);
}

//////// depth resources ////////
/////////////////////////////////
void create_depth_resources(_app *p_app) {
	VkFormat depth_format = find_depth_format(p_app, p_app->device.physical);

	create_image(p_app, &p_app->depth.image, 1, p_app->device.msaa_samples, &p_app->depth.image_allocation, p_app->swp.extent.width, p_app->swp.extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
	create_image_view(p_app, p_app->depth.image, &p_app->depth.image_view, 1, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
	transition_image_layout(p_app, p_app->depth.image, 1, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

//////// create framebuffers ////////
/////////////////////////////////////
void create_framebuffers(_app *p_app) {
	p_app->pipeline.swapchain_framebuffers = malloc(sizeof(VkFramebuffer) * p_app->swp.images_count);

	VkFramebufferCreateInfo framebuffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = p_app->pipeline.render_pass,
		.width = p_app->swp.extent.width,
		.height = p_app->swp.extent.height,
		.layers = 1,
	};


	VkImageView attachments[] = {
		p_app->colour.image_view,
		p_app->depth.image_view,
		NULL,
	};

	for (size_t i = 0; i < p_app->swp.images_count; i++) {

		attachments[2] = p_app->swp.image_views[i];

		framebuffer_create_info.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
		framebuffer_create_info.pAttachments = attachments;

		if (vkCreateFramebuffer(p_app->device.logical, &framebuffer_create_info, NULL, &p_app->pipeline.swapchain_framebuffers[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"framebuffers => failed to create framebuffers"
			);
			exit(EXIT_FAILURE);
		}
	}
}


//////// texture ////////
/////////////////////////
//// create mipmaps ////
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

//// create texture ////
void create_texture_image(_app *p_app) {
	p_app->tex.images = malloc(sizeof(VkImage) * p_app->obj.texture_count);
	p_app->tex.image_allocations = malloc(sizeof(VmaAllocation) * p_app->obj.texture_count);
	p_app->tex.image_views = malloc(sizeof(VkImageView) * p_app->obj.texture_count);
	p_app->tex.has_alpha = calloc(p_app->obj.texture_count, sizeof(u32));
	p_app->tex.mip_levels = malloc(sizeof(u32) * p_app->obj.texture_count);

	for (u32 i = 0; i < p_app->obj.texture_count; i++) {
		const char *path = p_app->obj.textures[i].path;

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
		p_app->tex.mip_levels[i] = (u32)floor(log2(tex_w > tex_h ? tex_w : tex_h)) + 1;

		u32 pixel_count = (u32)(tex_w * tex_h);
		for (u32 j = 0; j < pixel_count; j++) {
			if (pixels[j * 4 + 3] < 255) {
				p_app->tex.has_alpha[i] = 1;
				break;
			}
		}

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

//// texture image view ////
void create_texture_image_view(_app *p_app) {
	for (u32 i = 0; i < p_app->obj.texture_count; i++) {
		if (p_app->tex.images[i] != VK_NULL_HANDLE) {
			create_image_view(p_app, p_app->tex.images[i], &p_app->tex.image_views[i], p_app->tex.mip_levels[i], VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}
}

//// sampler ////
void create_texture_sampler(_app *p_app) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(p_app->device.physical, &properties);

	int max = 0;
	for (int i = 0; i < p_app->obj.texture_count; i++) {
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

//// create image ////
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

//// transition image layout ////
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

//// copy buffer to image ////
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

//////// buffers ////////
/////////////////////////

//// create buffer ////
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

//// copy buffer ////
void copy_buffer(_app *p_app, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
	VkCommandBuffer command_buffer = begin_single_time_commands(p_app);

	VkBufferCopy copy_region = {
		.size = size,
	};
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

	end_single_time_commands(p_app, command_buffer);
}

//// create billboard buffer ////
void create_billboard_buffer(_app *p_app) {
	u32 count = p_app->obj.billboard_count;

	VkDeviceSize buffer_size = sizeof(_billboard) * count;

	VkBuffer staging_buffer;
	VmaAllocation staging_alloc;
	create_buffer(p_app, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, &staging_buffer, &staging_alloc);

	void* data;
	vmaMapMemory(p_app->mem.alloc, staging_alloc, &data);
	memcpy(data, p_app->obj.billboards, buffer_size);
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

//// create mesh buffer ////
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


// create uniform buffers //
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

//////// descriptors ////////
/////////////////////////////
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

//////// command buffer ////////
////////////////////////////////
//// create command buffer ////
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

//// record command buffer ////
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
		vkCmdDrawIndexed(command_buffer, p_app->obj.indices_count[o], 1, 0, 0, 0);
	}

	free(transparent_draw_order);

if (p_app->obj.billboard_count > 0) {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline.billboard);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_app->billboard.instance_buffer, &offset);

    vkCmdDraw(command_buffer, 6, p_app->obj.billboard_count, 0, 0);
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

//////// sync objects ////////
//////////////////////////////
void create_sync_objects(_app *p_app) {

	p_app->sync.image_available_semaphores = malloc(sizeof(VkSemaphore) * p_app->swp.images_count);
	p_app->sync.render_finished_semaphores = malloc(sizeof(VkSemaphore) * p_app->swp.images_count);
	p_app->sync.in_flight_fences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	VkFenceCreateInfo fence_create_info = {	
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (u32 i = 0; i < p_app->swp.images_count; i++) {
		if (vkCreateSemaphore(p_app->device.logical, &semaphore_create_info, NULL, &p_app->sync.image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(p_app->device.logical, &semaphore_create_info, NULL, &p_app->sync.render_finished_semaphores[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"semaphore => failed to create semaphore"
			);
			exit(EXIT_FAILURE);
		}
	}

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateFence(p_app->device.logical, &fence_create_info, NULL, &p_app->sync.in_flight_fences[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst.instance,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"fence => failed to create fence"
			);
			exit(EXIT_FAILURE);
		}
	}
}

///////////////////////////////////////////
//////////////// MAIN LOOP ////////////////
///////////////////////////////////////////
//// loop ////
void main_loop(_app *p_app) {

	p_app->sync.frame_index = 0;

	while (!glfwWindowShouldClose(p_app->win.window)) {
		glfwPollEvents();
		update_view(p_app, get_delta_time());
		draw_frame(p_app);
	}

	vkDeviceWaitIdle(p_app->device.logical);
}

//// draw frame ////
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

//// clear swapchain ////
void cleanup_swapchain(_app *p_app) {
	for (size_t i = 0; i < p_app->swp.images_count; i++) {
		vkDestroyFramebuffer(p_app->device.logical, p_app->pipeline.swapchain_framebuffers[i], NULL);
		vkDestroyImageView(p_app->device.logical, p_app->swp.image_views[i], NULL);
	}
	free(p_app->pipeline.swapchain_framebuffers);
	free(p_app->swp.image_views);


	vkDestroyImageView(p_app->device.logical, p_app->depth.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->depth.image, p_app->depth.image_allocation);
	vkDestroyImageView(p_app->device.logical, p_app->colour.image_view, NULL);
	vmaDestroyImage(p_app->mem.alloc, p_app->colour.image, p_app->colour.image_allocation);

	vkDestroySwapchainKHR(p_app->device.logical, p_app->swp.swapchain, NULL);
}

//// recreate swapchain ////
void recreate_swapchain(_app *p_app) {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(p_app->win.window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(p_app->device.logical);

	cleanup_swapchain(p_app);

	create_swapchain(p_app);
	create_image_views(p_app);
	create_colour_resources(p_app);
	create_depth_resources(p_app);
	create_framebuffers(p_app);
}

//// update uniform buffer ////
void update_uniform_buffer(_app *p_app, u32 current_image) {
	static struct timespec start_time;
	static bool initialised = false;
	if (!initialised) {
		clock_gettime(CLOCK_MONOTONIC, &start_time);
		initialised = true;
	}

	struct timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	float time = (current_time.tv_sec - start_time.tv_sec) +
		(current_time.tv_nsec - start_time.tv_nsec) / 1e9f;

	_ubo ubo;

	glm_lookat(p_app->view.camera_pos, p_app->view.target, p_app->view.world_up, ubo.view);

	float aspect = (float)p_app->swp.extent.width / (float)p_app->swp.extent.height;
	glm_perspective(glm_rad(p_app->view.fov_y), aspect,
								 p_app->view.near_plane, p_app->view.far_plane, ubo.proj);
	ubo.proj[1][1] *= -1;

	vec4 light_pos = {0};
	glm_vec3_copy(p_app->obj.billboards[0].position, light_pos);
	light_pos[3] = 1.0f;
	glm_vec4_copy(light_pos, ubo.light_position);

	glm_vec4_copy((vec4){0.8f, 0.6f, 0.4f, 3.0f}, ubo.light_colour);
	glm_vec4_copy((vec4){1.0f, 1.0f, 1.0f, 0.0f}, ubo.ambient_light);

	memcpy(p_app->uniform.buffers_mapped[current_image], &ubo, sizeof(ubo));
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

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);

	if (p_app->view.first_mouse) {
		p_app->view.last_mouse_x = xpos;
		p_app->view.last_mouse_y = ypos;
		p_app->view.first_mouse = false;
		return;
	}

	float dx = xpos - p_app->view.last_mouse_x;
	float dy = p_app->view.last_mouse_y - ypos;

	p_app->view.last_mouse_x = xpos;
	p_app->view.last_mouse_y = ypos;

	dx *= p_app->view.sensitivity;
	dy *= p_app->view.sensitivity;

	p_app->view.yaw += dx;
	p_app->view.pitch += dy;

	if (p_app->view.pitch > 89.0f) p_app->view.pitch = 89.0f;
	if (p_app->view.pitch < -89.0f) p_app->view.pitch = -89.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	_app* p_app = (_app*)glfwGetWindowUserPointer(window);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !p_app->view.mouse_locked) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glfwGetCursorPos(window, &p_app->view.last_mouse_x, &p_app->view.last_mouse_y);

		p_app->view.first_mouse = true;
		p_app->view.mouse_locked = true;
	}
}

///////////////////////////////////////
//////////////// clean ////////////////
///////////////////////////////////////
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

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data) {

	if (strstr(p_callback_data->pMessage, "VK_KHR_portability_subset") != NULL || strstr(p_callback_data->pMessage, "vkGetPhysicalDeviceProperties2KHR") != NULL) {
		return VK_FALSE;
	}

	fprintf(stderr, "validation layer: %s\n", p_callback_data->pMessage);

	return VK_FALSE;
}

VkResult create_debug_utils_messenger_EXT(VkInstance inst, const VkDebugUtilsMessengerCreateInfoEXT *p_create_info, const VkAllocationCallbacks *p_alloc, VkDebugUtilsMessengerEXT *p_debug_messenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(inst, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL) {
		return func(inst, p_create_info, p_alloc, p_debug_messenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void destroy_debug_utils_messenger_EXT(VkInstance inst, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *p_alloc) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(inst, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL) {
		func(inst, debug_messenger, p_alloc);
	}
}

void setup_debug_messenger(_app *p_app) {
	if (!enable_validation_layers) return;

	VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
	populate_debug_messenger_create_info(&create_info);

	if (create_debug_utils_messenger_EXT(p_app->inst.instance, &create_info, NULL, &p_app->inst.debug_messenger) != VK_SUCCESS) {
		printf("failed to setup debug messenger\n");
		exit(EXIT_FAILURE);
	}
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info) {
	create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info->pfnUserCallback = debug_callback;
};

void submit_debug_message(VkInstance inst, VkDebugUtilsMessageSeverityFlagBitsEXT severity, const char* fmt, ...) {
	if (!enable_validation_layers) return;

	char buffer[1024];

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	PFN_vkSubmitDebugUtilsMessageEXT func = 
		(PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(inst, "vkSubmitDebugUtilsMessageEXT");

	if (func != NULL) {
		func(inst,
			 severity,
			 VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
			 &(VkDebugUtilsMessengerCallbackDataEXT){
			 .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT,
			 .pMessage = buffer
			 });
	}
}

void flatten(_app_obj* p_obj) {

	u32 idx_offset = 0;

	for (u32 o = 0; o < p_obj->mesh->object_count; ++o) {
		fastObjGroup* obj = &p_obj->mesh->objects[o];

		u32 index_count = 0;
		for (u32 f = obj->face_offset; f < obj->face_offset + obj->face_count; ++f) {
			uint32_t fv = p_obj->mesh->face_vertices[f];
			if (fv >= 3)
				index_count += (fv - 2) * 3;
		}

		p_obj->vertices[o] = malloc(index_count * sizeof(_vertex));
		p_obj->indices[o] = malloc(index_count * sizeof(u32));
		p_obj->face_indices[o] = malloc(index_count * sizeof(u32));
		p_obj->material_indices[o] = malloc(index_count * sizeof(u32));
		p_obj->group_indices[o] = malloc(index_count * sizeof(u32));
		p_obj->object_indices[o] = malloc(index_count * sizeof(u32));
		p_obj->texture_indices[o] = malloc(index_count * sizeof(u32));

		p_obj->vertices_count[o] = 0;
		p_obj->indices_count[o] = 0;

		u32 table_size = 1024;
		while (table_size < index_count) table_size <<= 1;
		_node** table = calloc(table_size, sizeof(_node*));
		_node* n_pool = malloc(index_count * sizeof(_node));

		u32 pool_index = 0;

		for (u32 f = obj->face_offset; f < obj->face_offset + obj->face_count; ++f) {
			uint32_t fv = p_obj->mesh->face_vertices[f];
			if (fv < 3) continue;

			u32 mat = 0; 
			if (p_obj->mesh->face_materials) {
				mat = p_obj->mesh->face_materials[f];
			}

			for (u32 i = 1; i + 1 < fv; ++i) {
				fastObjIndex tri[3] = {
					p_obj->mesh->indices[idx_offset + 0],
					p_obj->mesh->indices[idx_offset + i],
					p_obj->mesh->indices[idx_offset + i + 1]
				};

				for (int t = 0; t < 3; ++t) {
					fastObjIndex idx = tri[t];
					_vertex v = {0};

					if (idx.p >= 0 && idx.p < p_obj->mesh->position_count) {
						v.pos[0] = p_obj->mesh->positions[3 * idx.p + 0];
						v.pos[2] = -p_obj->mesh->positions[3 * idx.p + 1];
						v.pos[1] = p_obj->mesh->positions[3 * idx.p + 2];
					}

					if (idx.t >= 0 && idx.t < p_obj->mesh->texcoord_count) {
						v.tex[0] = p_obj->mesh->texcoords[2 * idx.t + 0];
						v.tex[1] = p_obj->mesh->texcoords[2 * idx.t + 1];
					}

					if (idx.n >= 0 && idx.n < p_obj->mesh->normal_count) {
						v.norm[0] = p_obj->mesh->normals[3 * idx.n + 0];
						v.norm[2] = -p_obj->mesh->normals[3 * idx.n + 1];
						v.norm[1] = p_obj->mesh->normals[3 * idx.n + 2];
					}

					v.tex_index = p_obj->mesh->materials[mat].map_Kd - 1;

					u32 hash = 5381;
					unsigned char* bytes = (unsigned char*)&v;
					for (size_t j = 0; j < sizeof(_vertex); j++) {
						hash = ((hash << 5) + hash) + bytes[j];
					}
					hash &= (table_size - 1);

					_node* n = table[hash];
					u32 found_index = 0;
					int found = 0;

					while (n) {
						if (memcmp(&n->vertex, &v, sizeof(_vertex)) == 0) {
							found = 1;
							found_index = n->index;
							break;
						}
						n = n->next;
					}

					if (found) {
						p_obj->indices[o][p_obj->indices_count[o]] = found_index;
					} else {
						p_obj->vertices[o][p_obj->vertices_count[o]] = v;
						p_obj->indices[o][p_obj->indices_count[o]] = p_obj->vertices_count[o];

						_node* new_n = &n_pool[pool_index++];
						new_n->vertex = v;
						new_n->index = p_obj->vertices_count[o];
						new_n->next = table[hash];
						table[hash] = new_n;

						p_obj->vertices_count[o]++;
					}

					fastObjMaterial *material = &p_obj->mesh->materials[mat];

					u32 texture_index = material->map_Kd;

					if (texture_index > 0 && texture_index <= p_obj->mesh->texture_count) {
						p_obj->texture_indices[o][p_obj->indices_count[o]] = texture_index;
					} else {
						p_obj->texture_indices[o][p_obj->indices_count[o]] = 0;
					}

					p_obj->face_indices[o][p_obj->indices_count[o]] = f;
					p_obj->material_indices[o][p_obj->indices_count[o]] = mat;
					p_obj->group_indices[o][p_obj->indices_count[o]] = o;
					p_obj->object_indices[o][p_obj->indices_count[o]] = o;

					p_obj->indices_count[o]++;
				}
			}

			idx_offset += fv;
		}

		free(table);
		free(n_pool);
	}
}
