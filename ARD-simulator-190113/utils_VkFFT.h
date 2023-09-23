#pragma once

#include "vkFFT.h"
#include <vector>

struct VkGPU
{
	uint64_t device_id;													// an id of a device, reported by Vulkan device list
	VkInstance instance;												// a connection between the application and the Vulkan library 
	VkPhysicalDevice physicalDevice;									// a handle for the graphics card used in the application
	VkPhysicalDeviceProperties physicalDeviceProperties;				// basic device properties
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;	// basic memory properties of the device
	VkDevice device;													// a logical device, interacting with physical device
	VkDebugUtilsMessengerEXT debugMessenger;							// extension for debugging
	uint64_t queueFamilyIndex;											// if multiple queues are available, specify the used one
	VkQueue queue;														// a place, where all operations are submitted
	VkCommandPool commandPool;											// an opaque objects that command buffer memory is allocated from
	VkFence fence;														// a vkGPU->fence used to synchronize dispatches
	std::vector<const char*> enabledDeviceExtensions;
	uint64_t enableValidationLayers;
};

// an example structure used to pass user-defined system for benchmarking
struct VkFFTUserSystemParameters
{
	uint64_t X;
	uint64_t Y;
	uint64_t Z;
	uint64_t P;
	uint64_t B;
	uint64_t N;
	uint64_t R2C;
	uint64_t DCT;
	uint64_t saveApplicationToString;
	uint64_t loadApplicationFromString;
};

class VkFFT_DCT
{
public:
	VkFFT_DCT(VkGPU* vkGPU, int dctType, int width, int height, int depth, float* input, float* output);
	~VkFFT_DCT();

	VkFFTResult execute();

private:
	VkGPU*				m_vkGPU{nullptr};
	VkFFTApplication	m_application{};
	VkFFTLaunchParams	m_launchParams{};
	int					m_width;
	int					m_height;
	int					m_depth;
	int					m_dctType;
	float*				m_input{ nullptr };
	float*				m_output{ nullptr };
	uint64_t			m_bufferSize{ 0 };
	VkBuffer			m_buffer{ VK_NULL_HANDLE };
	VkDeviceMemory		m_bufferDeviceMemory{ VK_NULL_HANDLE };
	double				m_time{ 0.0 };	// last execution time
};

VkResult CreateDebugUtilsMessengerEXT(VkGPU* vkGPU, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkGPU* vkGPU, const VkAllocationCallbacks* pAllocator);
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
VkResult setupDebugMessenger(VkGPU* vkGPU);
VkResult checkValidationLayerSupport();
std::vector<const char*> getRequiredExtensions(VkGPU* vkGPU, uint64_t sample_id);
VkResult createInstance(VkGPU* vkGPU, uint64_t sample_id);
VkResult findPhysicalDevice(VkGPU* vkGPU);
VkResult getComputeQueueFamilyIndex(VkGPU* vkGPU);
VkResult createDevice(VkGPU* vkGPU, uint64_t sample_id);
VkResult createFence(VkGPU* vkGPU);
VkResult createCommandPool(VkGPU* vkGPU);
VkFFTResult findMemoryType(VkGPU* vkGPU, uint64_t memoryTypeBits, uint64_t memorySize, VkMemoryPropertyFlags properties, uint32_t* memoryTypeIndex);
VkFFTResult allocateBuffer(VkGPU* vkGPU, VkBuffer* buffer, VkDeviceMemory* deviceMemory, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, uint64_t size);

VkFFTResult transferDataToCPU(VkGPU* vkGPU, void* cpu_arr, void* output_buffer, uint64_t bufferSize);
VkFFTResult transferDataFromCPU(VkGPU* vkGPU, void* cpu_arr, void* input_buffer, uint64_t bufferSize);
VkFFTResult devices_list();
VkFFTResult performVulkanFFT(VkGPU* vkGPU, VkFFTApplication* app, VkFFTLaunchParams* launchParams, int inverse, uint64_t num_iter);
VkFFTResult performVulkanFFTiFFT(VkGPU* vkGPU, VkFFTApplication* app, VkFFTLaunchParams* launchParams, uint64_t num_iter, double* time_result);

VkFFTResult initVkGPU(VkGPU* vkGPU);
VkFFTResult destroyVkGPU(VkGPU* vkGPU);

VkFFTResult initVkFFT_DCT(VkGPU* vkGPU, VkFFTApplication* app, int dctType, int width, int height, int depth, float* input, float* output);