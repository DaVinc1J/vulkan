#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <cglm/cglm.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "vk_mem_alloc.h"
#ifdef __cplusplus
}
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

const u32 validation_layer_count = 1;
const char* validation_layers[] = { "VK_LAYER_KHRONOS_validation" };
const char* enabled_logical_device_extensions[] = {
	"VK_KHR_swapchain",
	"VK_KHR_portability_subset",
};
const u32 enabled_logical_device_extensions_count = sizeof(enabled_logical_device_extensions) / sizeof(enabled_logical_device_extensions[0]);

#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif

const char* WIN_TITLE = "davinci";
const u32 WIN_WIDTH = 800;
const u32 WIN_HEIGHT = 800;

static const u32 MAX_FRAMES_IN_FLIGHT = 2;

typedef struct _vertex {
	vec2 pos;
	vec3 colour;
} _vertex;

const _vertex vertices[] = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
const u32 vertices_count = 4;

const u32 indices[] = {
	0, 1, 2, 2, 3, 0
};
const u32 indices_count = 6;

const u32 number_of_attributes = 2;

typedef struct _candidates {
	VkPhysicalDevice *p_physical_device;
	u32 score;
} _candidates;

typedef struct _ubo {
	mat4 model;
	mat4 view;
	mat4 proj;
} _ubo;

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

typedef struct _app {
	GLFWwindow *window;
	VkInstance inst;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkSurfaceKHR surface;
	_queue_family_indices queue_indices;
	VkSwapchainKHR swapchain;
	VkImage *swapchain_images;
	u32 swapchain_images_count;
	VkImageView *swapchain_image_views;
	VkSurfaceFormatKHR surface_format;
	VkExtent2D extent;
	VkRenderPass render_pass;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;
	VkFramebuffer *swapchain_framebuffers;
	VkCommandPool command_pool;
	VkCommandBuffer *command_buffers;
	VkSemaphore *image_available_semaphores;
	VkSemaphore *render_finished_semaphores;
	VkFence *in_flight_fences;
	u32 frame_index;
	bool framebuffer_resized;
	VkBuffer mesh_buffer;
	VmaAllocation mesh_allocation;
	u32 index_offset;
	VmaAllocator alloc;
	VkBuffer *uniform_buffers;
	VmaAllocation *uniform_buffer_allocations;
	void **uniform_buffers_mapped;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet *descriptor_sets;
} _app;

//// HELPER FUNCTIONS ////
// clamp //
u32 clamp(u32 n, u32 min, u32 max) {
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

// initial debug callback and messenger utils //
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
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

// latter validation layer connection functions that require app struct //
void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);
void setup_debug_messenger(_app *p_app);
void submit_debug_message(VkInstance inst, VkDebugUtilsMessageSeverityFlagBitsEXT severity, const char* fmt, ...);

//// HELPER FUNCTIONS ////

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
void create_image_views(_app *p_app);

// render pass //
void create_render_pass(_app *p_app);

// descriptor set layout //
void create_descriptor_set_layout(_app *p_app);

// graphics pipeline //
char* read_file(_app *p_app, const char *filename, size_t* shader_code_size);
void create_graphics_pipeline(_app *p_app);
VkShaderModule create_shader_module(_app *p_app, const char* shader_code, size_t shader_code_size);

// framebuffers //
void create_framebuffers(_app *p_app);

// command pool //
void create_command_pool(_app *p_app);

// buffers //
void create_buffer(_app *p_app, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, VkBuffer *p_buffer, VmaAllocation *p_allocation);
void copy_buffer(_app *p_app, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
void create_combined_mesh_buffer(_app *p_app);
void create_uniform_buffers(_app *p_app);

// descriptors //
void create_descriptor_pool(_app *p_app);
void create_descriptor_sets(_app *p_app);

// command buffer //
void create_command_buffers(_app *p_app);
void record_command_buffer(_app *p_app, VkCommandBuffer command_buffer, uint32_t image_index);

// sync objects //
void create_sync_objects(_app *p_app);

//// VULKAN INIT ////

//// MAIN LOOP ////
void main_loop(_app *p_app);
void draw_frame(_app *p_app);
void cleanup_swapchain(_app *p_app);
void recreate_swapchain(_app *p_app);
void update_uniform_buffer(_app *p_app, u32 current_image);
//// MAIN LOOP ////

//// CLEAN UP ////
void clean(_app *p_app);
//// CLEAN UP ////

//// VERTEX READ ////

VkVertexInputBindingDescription get_binding_description() {
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(_vertex);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_description;
}

void get_attribute_descriptions(VkVertexInputAttributeDescription* attribs, u32 *num_attribs) {
	if (attribs == NULL) {
		*num_attribs = number_of_attributes;
		return;
	}

	attribs[0].binding = 0;
	attribs[0].location = 0;
	attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribs[0].offset = offsetof(_vertex, pos);

	attribs[1].binding = 0;
	attribs[1].location = 1;
	attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attribs[1].offset = offsetof(_vertex, colour);
}

//// VERTEX READ ////


///////////////////////////////////////////
//////////////// MAIN LOOP ////////////////
///////////////////////////////////////////
int main() {
	_app app = {0};
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

	p_app->window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, NULL, NULL);

	glfwSetWindowUserPointer(p_app->window, p_app);
	glfwSetFramebufferSizeCallback(p_app->window, framebuffer_resize_callback);
}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
	_app *p_app = (_app*)glfwGetWindowUserPointer(window);
	p_app->framebuffer_resized = true;
}

/////////////////////////////////////////////
//////////////// VULKAN INIT ////////////////
/////////////////////////////////////////////

//////// vulkan main loop ////////
//////////////////////////////////
void vulkan_init(_app *p_app) {
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
	create_graphics_pipeline(p_app);
	create_framebuffers(p_app);
	create_command_pool(p_app);
	create_combined_mesh_buffer(p_app);
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
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"validation layers requested but no available"
		);
		exit(EXIT_FAILURE);
	}

	u32 glfw_extensions_count = 0;
	const char** glfw_extensions;

	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

	u32 required_extensions_count = glfw_extensions_count + (enable_validation_layers ? 2: 1);
	const char *required_extensions[required_extensions_count];

	for (u32 i = 0; i < glfw_extensions_count; i++) {
		required_extensions[i] = glfw_extensions[i];
	}

	required_extensions[glfw_extensions_count] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

	if (enable_validation_layers) {
		required_extensions[glfw_extensions_count + 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}


	u32 api_version = 0;
	vkEnumerateInstanceVersion(&api_version);

	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = WIN_TITLE,
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = WIN_TITLE,
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

	create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

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

	if (vkCreateInstance(&create_info, NULL, &p_app->inst) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"instance => failed to create instance"
		);
		exit(EXIT_FAILURE);
	};

	u32 extensions_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extensions_count, NULL);
	VkExtensionProperties extensions[extensions_count];
	vkEnumerateInstanceExtensionProperties(NULL, &extensions_count, extensions);

	if (verify_extensions_support(extensions_count, extensions, glfw_extensions_count, glfw_extensions)) {
	} else {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"instance => extensions support verify failed"
		);
		exit(EXIT_FAILURE);
	}
}

//// create surface ////
void create_surface(_app *p_app) {
	if (glfwCreateWindowSurface(p_app->inst, p_app->window, NULL, &p_app->surface) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
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

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, p_app->surface, &p_support->capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, p_app->surface, &p_support->surface_formats_count, NULL);
	if (p_support->surface_formats_count != 0) {
		p_support->surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * p_support->surface_formats_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, p_app->surface, &p_support->surface_formats_count, p_support->surface_formats);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, p_app->surface, &p_support->present_modes_count, NULL);
	if (p_support->present_modes_count != 0) {
		p_support->present_modes = malloc(sizeof(VkPresentModeKHR) * p_support->present_modes_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, p_app->surface, &p_support->present_modes_count, p_support->present_modes);
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
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, p_app->surface, &present_support);
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
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
			"physical device => lacks geometry shader support"
		);
	}

	if (!physical_device_features.samplerAnisotropy) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => lacks sampler anisotropy"
		);
		return false;
	}

	_queue_family_indices indices = find_queue_families(p_app, physical_device);
	if (indices.is_graphics_family_set == 0 || indices.is_present_family_set == 0) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find queue families"
		);
		return false;
	}

	if (!is_physical_device_extensions_supported(physical_device)) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => extensions not supported"
		);
		return false;
	}

	_swapchain_support support;
	if (!check_swapchain_support(p_app, physical_device, &support)) {
		submit_debug_message(
			p_app->inst,
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
void pick_physical_device(_app *p_app) {
	p_app->physical_device = VK_NULL_HANDLE;

	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(p_app->inst, &physical_device_count, NULL);

	if (physical_device_count == 0) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find any devices"
		);
		exit(EXIT_FAILURE);
	}

	VkPhysicalDevice physical_devices[physical_device_count];
	vkEnumeratePhysicalDevices(p_app->inst, &physical_device_count, physical_devices);

	if (physical_device_count == 1) {
		if (!is_physical_device_suitable(p_app, physical_devices[0])) {
			submit_debug_message(
				p_app->inst,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"physical device => only device is not suitable"
			);
			exit(EXIT_FAILURE);
		}

		p_app->physical_device = physical_devices[0];
		submit_debug_message(
			p_app->inst,
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
		p_app->physical_device = *candidates[max_index].p_physical_device;
	}

	if (p_app->physical_device == VK_NULL_HANDLE) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"physical device => failed to find a suitable device"
		);
		exit(EXIT_FAILURE);
	}
}

//////// create logical device ////////
///////////////////////////////////////
void create_logical_device(_app *p_app) {
	p_app->queue_indices = find_queue_families(p_app, p_app->physical_device);

	u32 unique_queue_families[2] = {
		p_app->queue_indices.graphics_family,
		p_app->queue_indices.present_family,
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
	vkGetPhysicalDeviceFeatures(p_app->physical_device, &physical_device_features);
	physical_device_features.robustBufferAccess = VK_FALSE;

	VkDeviceCreateInfo logical_device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = queue_create_info_count,
		.pQueueCreateInfos = queue_create_infos,
		.pEnabledFeatures = &physical_device_features,
		.enabledExtensionCount = enabled_logical_device_extensions_count,
		.ppEnabledExtensionNames = enabled_logical_device_extensions,
	};

	logical_device_create_info.enabledLayerCount = 0;

	if (enable_validation_layers) {
		logical_device_create_info.enabledLayerCount = validation_layer_count;
		logical_device_create_info.ppEnabledLayerNames = validation_layers;
	}

	if (vkCreateDevice(p_app->physical_device, &logical_device_create_info, NULL, &p_app->logical_device) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"logical device => failed to create logical device"
		);
		exit(EXIT_FAILURE);
	}

	vkGetDeviceQueue(p_app->logical_device, p_app->queue_indices.graphics_family, 0, &p_app->graphics_queue);
	vkGetDeviceQueue(p_app->logical_device, p_app->queue_indices.present_family, 0, &p_app->present_queue);
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
		.physicalDevice = p_app->physical_device,
		.device = p_app->logical_device,
		.instance = p_app->inst,
		.pVulkanFunctions = &vulkan_funcs,
	};

	if (vmaCreateAllocator(&allocatorCreateInfo, &p_app->alloc) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst, 
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
		p_app->inst,
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
		p_app->inst,
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
		glfwGetFramebufferSize(p_app->window, &width, &height);

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
	check_swapchain_support(p_app, p_app->physical_device, &support);

	p_app->surface_format = choose_swapchain_surface_format(p_app, &support);
	VkPresentModeKHR present_mode = choose_swapchain_present_mode(p_app, &support);
	p_app->extent = choose_swapchain_swap_extent(p_app, &support);

	u32 image_count = support.capabilities.minImageCount + 1;
	if (support.capabilities.maxImageCount > 0 && image_count > support.capabilities.maxImageCount) {
		image_count = support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = p_app->surface,
		.minImageCount = image_count,
		.imageFormat = p_app->surface_format.format,
		.imageColorSpace = p_app->surface_format.colorSpace,
		.imageExtent = p_app->extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = support.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	u32 queue_family_indices[] = {p_app->queue_indices.graphics_family, p_app->queue_indices.present_family};

	if (p_app->queue_indices.graphics_family != p_app->queue_indices.present_family) {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = NULL;
	}

	if (vkCreateSwapchainKHR(p_app->logical_device, &swapchain_create_info, NULL, &p_app->swapchain) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to create swapchain"
		);
	}

	p_app->swapchain_images_count = 0;
	vkGetSwapchainImagesKHR(p_app->logical_device, p_app->swapchain, &p_app->swapchain_images_count, NULL);
	p_app->swapchain_images = malloc(sizeof(VkImage) * p_app->swapchain_images_count);
	vkGetSwapchainImagesKHR(p_app->logical_device, p_app->swapchain, &p_app->swapchain_images_count, p_app->swapchain_images);
}

//////// create image views ////////
////////////////////////////////////
void create_image_views(_app *p_app) {
	p_app->swapchain_image_views = malloc(sizeof(VkImageView) * p_app->swapchain_images_count);


	VkImageViewCreateInfo image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = p_app->surface_format.format,
		.components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1,
	};

	for (u32 i = 0; i < p_app->swapchain_images_count; i++) {
		image_view_create_info.image = p_app->swapchain_images[i];
		if (vkCreateImageView(p_app->logical_device, &image_view_create_info, NULL, &p_app->swapchain_image_views[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"image views => failed to create image view"
			);
		}
	}
}

//////// create render pass ////////
////////////////////////////////////
void create_render_pass(_app *p_app) {
	VkAttachmentDescription colour_attachment_description = {
		.format = p_app->surface_format.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};


	VkAttachmentReference colour_attachment_reference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colour_attachment_reference,
	};


	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
	};

	VkRenderPassCreateInfo render_pass_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colour_attachment_description,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	if (vkCreateRenderPass(p_app->logical_device, &render_pass_create_info, NULL, &p_app->render_pass) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
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
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = NULL,
	};

	VkDescriptorSetLayoutCreateInfo ubo_layout_create_info = {

		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = &ubo_layout_binding,
	};

	if (vkCreateDescriptorSetLayout(p_app->logical_device, &ubo_layout_create_info, NULL, &p_app->descriptor_set_layout) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
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
			p_app->inst,
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
			p_app->inst,
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
void create_graphics_pipeline(_app *p_app) {
	size_t vert_shader_code_size;
	size_t frag_shader_code_size;
	const char* vert_shader_code = read_file(p_app, "src/shaders/vert.spv", &vert_shader_code_size);
	const char* frag_shader_code = read_file(p_app, "src/shaders/frag.spv", &frag_shader_code_size);

	VkShaderModule vert_shader_module = create_shader_module(p_app, vert_shader_code, vert_shader_code_size); 
	VkShaderModule frag_shader_module = create_shader_module(p_app, frag_shader_code, frag_shader_code_size);

	u32 attribute_count = 0;
	get_attribute_descriptions(NULL, &attribute_count);
	VkVertexInputAttributeDescription attribute_descriptions[attribute_count];
	get_attribute_descriptions(attribute_descriptions, NULL);
	VkVertexInputBindingDescription binding_description = get_binding_description();

	VkPipelineShaderStageCreateInfo vert_shader_stage_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vert_shader_module,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo frag_shader_stage_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = frag_shader_module,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo shaderStages[] = {vert_shader_stage_create_info, frag_shader_stage_create_info};

	VkDynamicState dynamic_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	u32 dynamic_states_size = (sizeof(dynamic_states) / sizeof(VkDynamicState));

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = dynamic_states_size,
		.pDynamicStates = dynamic_states,
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.vertexAttributeDescriptionCount = attribute_count,
		.pVertexBindingDescriptions = &binding_description,
		.pVertexAttributeDescriptions = attribute_descriptions,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (float) p_app->extent.width,
		.height = (float) p_app->extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = p_app->extent,
	};

	VkPipelineViewportStateCreateInfo viewport_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};


	VkPipelineRasterizationStateCreateInfo rasterizer_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisampling_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable = VK_FALSE,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading = 1.0f,
		.pSampleMask = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState colour_blend_attachment_state = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendStateCreateInfo colour_blend_state_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colour_blend_attachment_state,
		.blendConstants[0] = 0.0f,
		.blendConstants[1] = 0.0f,
		.blendConstants[2] = 0.0f,
		.blendConstants[3] = 0.0f,
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &p_app->descriptor_set_layout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = NULL,
	};

	if (vkCreatePipelineLayout(p_app->logical_device, &pipeline_layout_create_info, NULL, &p_app->pipeline_layout) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"pipeline layout => failed to create pipeline layout"
		);
		exit(EXIT_FAILURE);
	}

	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertex_input_state_create_info,
		.pInputAssemblyState = &input_assembly_state_create_info,
		.pViewportState = &viewport_state_create_info,
		.pRasterizationState = &rasterizer_state_create_info,
		.pMultisampleState = &multisampling_state_create_info,
		.pDepthStencilState = NULL,
		.pColorBlendState = &colour_blend_state_create_info,
		.pDynamicState = &dynamic_state_create_info,
		.layout = p_app->pipeline_layout,
		.renderPass = p_app->render_pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	if (vkCreateGraphicsPipelines(p_app->logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &p_app->pipeline) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"pipeline => failed to create pipeline"
		);
		exit(EXIT_FAILURE);
	}

	vkDestroyShaderModule(p_app->logical_device, frag_shader_module, NULL);
	vkDestroyShaderModule(p_app->logical_device, vert_shader_module, NULL);
}

//// create shader module ////
VkShaderModule create_shader_module(_app *p_app, const char* shader_code, size_t shader_code_size) {
	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shader_code_size,
		.pCode = (const uint32_t*) shader_code,
	};

	VkShaderModule shader_module;
	if (vkCreateShaderModule(p_app->logical_device, &shader_module_create_info, NULL, &shader_module) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"shader module => failed to create shader module"
		);
		exit(EXIT_FAILURE);
	}

	return shader_module;
}


//////// create framebuffers ////////
/////////////////////////////////////
void create_framebuffers(_app *p_app) {
	p_app->swapchain_framebuffers = malloc(sizeof(VkFramebuffer) * p_app->swapchain_images_count);

	VkFramebufferCreateInfo framebuffer_create_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = p_app->render_pass,
		.attachmentCount = 1,
		.width = p_app->extent.width,
		.height = p_app->extent.height,
		.layers = 1,
	};

	for (size_t i = 0; i < p_app->swapchain_images_count; i++) {
		VkImageView attachments[] = {
			p_app->swapchain_image_views[i]
		};

		framebuffer_create_info.pAttachments = attachments;

		if (vkCreateFramebuffer(p_app->logical_device, &framebuffer_create_info, NULL, &p_app->swapchain_framebuffers[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"framebuffers => failed to create framebuffers"
			);
			exit(EXIT_FAILURE);
		}
	}
}

//////// command pool ////////
//////////////////////////////
void create_command_pool(_app *p_app) {
	VkCommandPoolCreateInfo command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = p_app->queue_indices.graphics_family,
	};

	if (vkCreateCommandPool(p_app->logical_device, &command_pool_create_info, NULL, &p_app->command_pool) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"command pool => failed to create command pool"
		);
		exit(EXIT_FAILURE);
	}
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

	if (vmaCreateBuffer(p_app->alloc, &buffer_create_info, &alloc_info, p_buffer, p_allocation, NULL) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"vma => failed to create buffer"
		);
		exit(EXIT_FAILURE);
	}
}

//// copy buffer ////
void copy_buffer(_app *p_app, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
	VkCommandBufferAllocateInfo buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = p_app->command_pool,
		.commandBufferCount = 1,
	};

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(p_app->logical_device, &buffer_alloc_info, &command_buffer);

	VkCommandBufferBeginInfo buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(command_buffer, &buffer_begin_info);

	VkBufferCopy buffer_copy_region = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size,
	};
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &buffer_copy_region);

	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer,
	};

	vkQueueSubmit(p_app->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(p_app->graphics_queue);
	vkFreeCommandBuffers(p_app->logical_device, p_app->command_pool, 1, &command_buffer);

}

//// create combined_buffer ////
void create_combined_mesh_buffer(_app *p_app) {
	VkDeviceSize vertex_size = sizeof(vertices[0]) * vertices_count;
	VkDeviceSize index_size = sizeof(indices[0]) * indices_count;
	VkDeviceSize total_size = vertex_size + index_size;

	p_app->index_offset = (u32)vertex_size;

	VkBuffer staging_buffer;
	VmaAllocation staging_allocation;
	create_buffer(p_app, total_size,
							 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							 VMA_MEMORY_USAGE_CPU_ONLY,
							 &staging_buffer, &staging_allocation);

	void *data;
	vmaMapMemory(p_app->alloc, staging_allocation, &data);
	memcpy(data, vertices, (size_t)vertex_size);
	memcpy((char *)data + vertex_size, indices, (size_t)index_size);
	vmaUnmapMemory(p_app->alloc, staging_allocation);

	create_buffer(p_app, total_size,
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
							 VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							 VMA_MEMORY_USAGE_GPU_ONLY,
							 &p_app->mesh_buffer,
							 &p_app->mesh_allocation);

	copy_buffer(p_app, staging_buffer, p_app->mesh_buffer, total_size);
	vmaDestroyBuffer(p_app->alloc, staging_buffer, staging_allocation);
}


// create uniform buffers //
void create_uniform_buffers(_app *p_app) {
	VkDeviceSize buffer_size = sizeof(_ubo);

	p_app->uniform_buffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
	p_app->uniform_buffer_allocations = malloc(sizeof(VmaAllocation) * MAX_FRAMES_IN_FLIGHT);
	p_app->uniform_buffers_mapped = malloc(sizeof(void*) * MAX_FRAMES_IN_FLIGHT);

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
		if (vmaCreateBuffer(p_app->alloc, &buffer_create_info, &alloc_create_info,
											&p_app->uniform_buffers[i],
											&p_app->uniform_buffer_allocations[i],
											&allocation_info) != VK_SUCCESS) {

			submit_debug_message(
				p_app->inst,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"uniform buffers => failed to create uniforms buffers"
			);
			exit(1);
			exit(EXIT_FAILURE);
		}

		p_app->uniform_buffers_mapped[i] = allocation_info.pMappedData;
	}
}

//////// descriptors ////////
/////////////////////////////
void create_descriptor_pool(_app *p_app) {
	VkDescriptorPoolSize pool_size = {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = (u32)MAX_FRAMES_IN_FLIGHT,
	};

	VkDescriptorPoolCreateInfo pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
		.maxSets = (u32)MAX_FRAMES_IN_FLIGHT,
	};

	if (vkCreateDescriptorPool(p_app->logical_device, &pool_create_info, NULL, &p_app->descriptor_pool) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"descriptor pool => failed to create descriptor pool"
		);
		exit(EXIT_FAILURE);
}

}

void create_descriptor_sets(_app *p_app) {
	p_app->descriptor_sets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);

	VkDescriptorSetLayout *layouts = malloc(sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = p_app->descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = p_app->descriptor_pool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts
    };

    if (vkAllocateDescriptorSets(p_app->logical_device, &alloc_info, p_app->descriptor_sets) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"descriptor sets => failed to allocate descriptor sets"
		);
        exit(EXIT_FAILURE);
    }

	    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info = {
            .buffer = p_app->uniform_buffers[i],
            .offset = 0,
            .range = sizeof(_ubo)
        };

        VkWriteDescriptorSet descriptor_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = p_app->descriptor_sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &buffer_info,
            .pImageInfo = NULL,
            .pTexelBufferView = NULL
        };

        vkUpdateDescriptorSets(p_app->logical_device, 1, &descriptor_write, 0, NULL);
    }

    free(layouts);

}

//////// command buffer ////////
////////////////////////////////
//// create command buffer ////
void create_command_buffers(_app *p_app) {

	p_app->command_buffers = malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo command_buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = p_app->command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
	};

	if (vkAllocateCommandBuffers(p_app->logical_device, &command_buffer_alloc_info, p_app->command_buffers) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"command buffer => failed to allocate command buffer"
		);
		exit(EXIT_FAILURE);
	}
}

//// record command buffer ////
void record_command_buffer(_app *p_app, VkCommandBuffer command_buffer, uint32_t image_index) {
	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = 0,
		.pInheritanceInfo = NULL,
	};

	if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"command buffer => failed to begin record command buffer"
		);
		exit(EXIT_FAILURE);
	}

	VkClearValue clear_colour = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

	VkRenderPassBeginInfo render_pass_begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = p_app->render_pass,
		.framebuffer = p_app->swapchain_framebuffers[image_index],
		.renderArea.offset = {0, 0},
		.renderArea.extent = p_app->extent,
		.clearValueCount = 1,
		.pClearValues = &clear_colour,
	};

	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline);

	VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = (float) p_app->extent.width,
		.height = (float) p_app->extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = p_app->extent,
	};
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);            

	VkBuffer vertex_buffers[] = {p_app->mesh_buffer};

	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
	vkCmdBindIndexBuffer(command_buffer, p_app->mesh_buffer, p_app->index_offset, VK_INDEX_TYPE_UINT32);

	size_t vertices_count = sizeof(vertices) / sizeof(vertices[0]);

vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_app->pipeline_layout, 0, 1, &p_app->descriptor_sets[p_app->frame_index], 0, NULL);
	vkCmdDrawIndexed(command_buffer, indices_count, 1, 0, 0, 0);

	vkCmdEndRenderPass(command_buffer);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"command buffer => failed to end record command buffer"
		);
		exit(EXIT_FAILURE);
	}
}

//////// sync objects ////////
//////////////////////////////
void create_sync_objects(_app *p_app) {

	p_app->image_available_semaphores = malloc(sizeof(VkSemaphore) * p_app->swapchain_images_count);
	p_app->render_finished_semaphores = malloc(sizeof(VkSemaphore) * p_app->swapchain_images_count);
	p_app->in_flight_fences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	VkFenceCreateInfo fence_create_info = {	
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (u32 i = 0; i < p_app->swapchain_images_count; i++) {
		if (vkCreateSemaphore(p_app->logical_device, &semaphore_create_info, NULL, &p_app->image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(p_app->logical_device, &semaphore_create_info, NULL, &p_app->render_finished_semaphores[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst,
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				"semaphore => failed to create semaphore"
			);
			exit(EXIT_FAILURE);
		}
	}

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateFence(p_app->logical_device, &fence_create_info, NULL, &p_app->in_flight_fences[i]) != VK_SUCCESS) {
			submit_debug_message(
				p_app->inst,
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

	p_app->frame_index = 0;

	while (!glfwWindowShouldClose(p_app->window)) {
		glfwPollEvents();
		draw_frame(p_app);
	}

	vkDeviceWaitIdle(p_app->logical_device);
}

//// draw frame ////
void draw_frame(_app *p_app) {
	vkWaitForFences(p_app->logical_device, 1, &p_app->in_flight_fences[p_app->frame_index], VK_TRUE, UINT64_MAX);

	u32 image_index;
	VkResult aquire_result = vkAcquireNextImageKHR(
		p_app->logical_device,
		p_app->swapchain,
		UINT64_MAX,
		p_app->image_available_semaphores[p_app->frame_index],
		VK_NULL_HANDLE,
		&image_index
	);

	if (aquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swapchain(p_app);
		return;
	} else if (aquire_result != VK_SUCCESS && aquire_result != VK_SUBOPTIMAL_KHR) {

		printf("vkAcquireNextImageKHR returned: %d, image index: %u\n", aquire_result, image_index);

		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to acquire swap chain image"
		);
		exit(EXIT_FAILURE);
	}


	update_uniform_buffer(p_app, p_app->frame_index);

	vkResetFences(p_app->logical_device, 1, &p_app->in_flight_fences[p_app->frame_index]);

	vkResetCommandBuffer(p_app->command_buffers[p_app->frame_index], 0);
	record_command_buffer(p_app, p_app->command_buffers[p_app->frame_index], image_index);

	VkSemaphore wait_semaphores[] = {p_app->image_available_semaphores[p_app->frame_index]};
	VkSemaphore signal_semaphores[] = {p_app->render_finished_semaphores[image_index]};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &p_app->command_buffers[p_app->frame_index],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signal_semaphores,
	};

	if (vkQueueSubmit(p_app->graphics_queue, 1, &submit_info, p_app->in_flight_fences[p_app->frame_index]) != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"graphics queue => failed to submit next queue"
		);
		exit(EXIT_FAILURE);
	}

	VkSwapchainKHR swapchains[] = {p_app->swapchain};
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signal_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &image_index,
		.pResults = NULL,
	};

	VkResult present_result = vkQueuePresentKHR(p_app->present_queue, &present_info);


	if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR || p_app->framebuffer_resized) {
		p_app->framebuffer_resized = false;
		recreate_swapchain(p_app);
		return;
	} else if (present_result != VK_SUCCESS) {
		submit_debug_message(
			p_app->inst,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			"swapchain => failed to present swap chain image"
		);
		exit(EXIT_FAILURE);
	}

	p_app->frame_index = (p_app->frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

//// clear swapchain ////
void cleanup_swapchain(_app *p_app) {
	for (size_t i = 0; i < p_app->swapchain_images_count; i++) {
		vkDestroyFramebuffer(p_app->logical_device, p_app->swapchain_framebuffers[i], NULL);
		vkDestroyImageView(p_app->logical_device, p_app->swapchain_image_views[i], NULL);
	}
	free(p_app->swapchain_framebuffers);
	free(p_app->swapchain_image_views);

	vkDestroySwapchainKHR(p_app->logical_device, p_app->swapchain, NULL);
}

//// recreate swapchain ////
void recreate_swapchain(_app *p_app) {
	int width = 0, height = 0;
	glfwGetFramebufferSize(p_app->window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(p_app->window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(p_app->logical_device);

	cleanup_swapchain(p_app);

	create_swapchain(p_app);
	create_image_views(p_app);
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

	glm_mat4_identity(ubo.model);
	glm_rotate(ubo.model, glm_rad(90.0f) * time, (vec3){0.0f, 0.0f, 1.0f});

	glm_lookat((vec3){2.0f, 2.0f, 2.0f},
						(vec3){0.0f, 0.0f, 0.0f},
						(vec3){0.0f, 0.0f, 1.0f},
						ubo.view);

	glm_perspective(glm_rad(45.0f),
								 p_app->extent.width / (float)p_app->extent.height,
								 0.1f, 10.0f,
								 ubo.proj);

	ubo.proj[1][1] *= -1;

	memcpy(p_app->uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

///////////////////////////////////////
//////////////// clean ////////////////
///////////////////////////////////////
void clean(_app *p_app) {

	vkDestroyDescriptorPool(p_app->logical_device, p_app->descriptor_pool, NULL);
	vkDestroyDescriptorSetLayout(p_app->logical_device, p_app->descriptor_set_layout, NULL);
	free(p_app->descriptor_sets);
	p_app->descriptor_sets = NULL;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vmaDestroyBuffer(p_app->alloc, p_app->uniform_buffers[i], p_app->uniform_buffer_allocations[i]);
	}
	free(p_app->uniform_buffers);
	free(p_app->uniform_buffer_allocations);
	free(p_app->uniform_buffers_mapped);
	p_app->uniform_buffers = NULL;
	p_app->uniform_buffer_allocations = NULL;
	p_app->uniform_buffers_mapped = NULL;

	vmaDestroyBuffer(p_app->alloc, p_app->mesh_buffer, p_app->mesh_allocation);

	free(p_app->command_buffers);
	p_app->command_buffers = NULL;

	for (u32 i = 0; i < p_app->swapchain_images_count; i++) {
		vkDestroySemaphore(p_app->logical_device, p_app->image_available_semaphores[i], NULL);
		vkDestroySemaphore(p_app->logical_device, p_app->render_finished_semaphores[i], NULL);
	}

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(p_app->logical_device, p_app->in_flight_fences[i], NULL);
	}

	free(p_app->image_available_semaphores);
	p_app->image_available_semaphores = NULL;
	free(p_app->render_finished_semaphores);
	p_app->render_finished_semaphores = NULL;
	free(p_app->in_flight_fences);
	p_app->in_flight_fences = NULL;

	vkDestroyCommandPool(p_app->logical_device, p_app->command_pool, NULL);

	for (size_t i = 0; i < p_app->swapchain_images_count; i++) {
		vkDestroyFramebuffer(p_app->logical_device, p_app->swapchain_framebuffers[i], NULL);
	}
	free(p_app->swapchain_framebuffers);
	p_app->swapchain_framebuffers = NULL;

	vkDestroyPipeline(p_app->logical_device, p_app->pipeline, NULL);
	vkDestroyPipelineLayout(p_app->logical_device, p_app->pipeline_layout, NULL);

	vkDestroyDescriptorSetLayout(p_app->logical_device, p_app->descriptor_set_layout, NULL);

	vkDestroyRenderPass(p_app->logical_device, p_app->render_pass, NULL);	

	for (u32 i = 0; i < p_app->swapchain_images_count; i++) {
		vkDestroyImageView(p_app->logical_device, p_app->swapchain_image_views[i], NULL);
	}
	free(p_app->swapchain_image_views);
	p_app->swapchain_image_views = NULL;

	vkDestroySwapchainKHR(p_app->logical_device, p_app->swapchain, NULL);

	vmaDestroyAllocator(p_app->alloc);

	vkDestroyDevice(p_app->logical_device, NULL);

	if (enable_validation_layers) {
		destroy_debug_utils_messenger_EXT(p_app->inst, p_app->debug_messenger, NULL);
	}

	vkDestroySurfaceKHR(p_app->inst, p_app->surface, NULL);
	vkDestroyInstance(p_app->inst, NULL);
	glfwDestroyWindow(p_app->window);
	glfwTerminate();
}

/////////////////////////////////////////////////
//////////////// debug messenger ////////////////
/////////////////////////////////////////////////
void setup_debug_messenger(_app *p_app) {
	if (!enable_validation_layers) return;

	VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
	populate_debug_messenger_create_info(&create_info);

	if (create_debug_utils_messenger_EXT(p_app->inst, &create_info, NULL, &p_app->debug_messenger) != VK_SUCCESS) {
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
